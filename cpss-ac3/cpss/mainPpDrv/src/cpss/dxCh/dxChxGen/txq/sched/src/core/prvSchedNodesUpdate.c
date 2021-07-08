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
 * @brief tm_nodes_update implementation.
 *
* @file tm_nodes_update.c
*
* $Revision: 2.0 $
 */

/* #include <assert.h> */

#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedNodesUpdate.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrCodes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSetHwRegisters.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedSetLocalDbDefaults.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedHwConfigurationInterface.h>
/* for validation shaping profiles during node creation */
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedShapingInternal.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedShapingUtils.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedNodesCtlInternal.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvRmStatus.h>

#include <errno.h>
#include <stdlib.h>

int prvCpssSchedUpdateEligFunc(PRV_CPSS_SCHED_HANDLE hndl, uint8_t level, uint32_t index, uint8_t elig_func)
{
    int rc = 0;

    TM_CTL(ctl, hndl);

    CONVERT_TO_PHYSICAL(level, index); /* If supported - inside macro. */



	VALIDATE_ELIG_FUNCTION(elig_func)

    switch (level) {
    case Q_LEVEL:
        if (index >= ctl->tm_total_queues) {
            rc = -ENODATA;
            goto out;
        }
        ctl->tm_queue_array[index].elig_prio_func = elig_func;
        break;
    case A_LEVEL:
        if (index >= ctl->tm_total_a_nodes) {
            rc = -ENODATA;
            goto out;
        }
        ctl->tm_a_node_array[index].elig_prio_func = elig_func;
        break;
    case B_LEVEL:
        if (index >= ctl->tm_total_b_nodes) {
            rc = -ENODATA;
            goto out;
        }
        ctl->tm_b_node_array[index].elig_prio_func = elig_func;
        break;
    case C_LEVEL:
        if (index >= ctl->tm_total_c_nodes) {
            rc = -ENODATA;
            goto out;
        }
        ctl->tm_c_node_array[index].elig_prio_func = elig_func;
        break;
    case P_LEVEL:
        if (index >= ctl->tm_total_ports) {
            rc = -ENODATA;
            goto out;
        }
        ctl->tm_port_array[index].elig_prio_func = elig_func;
        break;
    default:
        rc = -EADDRNOTAVAIL;
        goto out;
    }

    if (rc >= 0) rc = prvSchedLowLevelSetNodeEligPriorityFunc(hndl, level, index);
	if (rc) goto out;
    
 #if 0   
	/* in some realizations PerEn  is implemented also per node 
	*  so changing eligible function can cause  to change  PerEn  value per current node
	*  It's possible to set node PerEn  there always enabled or update shaping parameters
	*  here  and node PerEn  will be updated there according to new eligible function type
	*/
	if (level==P_LEVEL)	rc=prvSchedLowLevelSetPortShapingParametersToHw(hndl, (uint8_t)index);
	else				rc=prvSchedLowLevelSetNodeShapingParametersToHwWithValidation(hndl, level, index);
#endif
out:
    return rc;
}

/***************************************************************************
 * Parameters Update
 ***************************************************************************/

/* 
UPDATE_ELIGIBLE_FUNCTION return OOR error in 2 cases: 
    1) out of range elig_func_index 
    2) shaping depended elig_func_index when shaping / periodic scheme is disabled 
       cause to traffic to stop.
*/
#define UPDATE_ELIGIBLE_FUNCTION(level , elig_func_table, elig_func_test, elig_func_index, elig_func_pointer , if_valid_action) \
    if (elig_func_index != TM_INVAL_8_BIT)\
	{\
        switch (elig_func_test(elig_func_table, elig_func_index))\
        {\
			case -1:rc = TM_CONF_ELIG_PRIO_FUNC_ID_OOR; goto out;\
			case  1:\
			if (ctl->level_data[level].shaping_status == TM_DISABLE)\
			{\
				rc = TM_CONF_ELIG_PRIO_FUNC_ID_OOR;\
				goto out;\
			}\
			default:\
			elig_func_pointer = elig_func_index;\
			if_valid_action;\
		}\
    }
#define     UPDATE_QUEUE_ELIGIBLE_FUNCTION( if_valid_action ) \
	UPDATE_ELIGIBLE_FUNCTION(Q_LEVEL,ctl->tm_elig_prio_q_lvl_tbl, prvSchedIsQueueEligFuncUseShaper , params->elig_prio_func_ptr, queue->elig_prio_func , if_valid_action )
#define     UPDATE_NODE_ELIGIBLE_FUNCTION(level, elig_func_table,  if_valid_action ) \
	UPDATE_ELIGIBLE_FUNCTION(level,elig_func_table, prvSchedIsNodeEligFuncUseShaper, params->elig_prio_func_ptr, node->elig_prio_func , if_valid_action )




#define	UPDATE_DWRR_ENABLE_PER_PRIORITY_MASK(dwrr_prio_flag_array, node, error, action) \
	for (i=0; i<8; i++)\
	{\
		if ( dwrr_prio_flag_array[i] != TM_INVAL_8_BIT)\
		{\
			if ((dwrr_prio_flag_array[i] != TM_DISABLE) && (dwrr_prio_flag_array[i] != TM_ENABLE))	\
			{\
				rc = error;\
				goto out;\
			}\
			node->dwrr_enable_per_priority_mask = (node->dwrr_enable_per_priority_mask & (~(0x1 << i)) & 0xFF )  /* reset bit in bitmask */\
												 | (dwrr_prio_flag_array[i] << i); \
			action\
		}\
	}
	
#define	UPDATE_NODE_DWRR_ENABLE_PER_PRIORITY_MASK(error) 	UPDATE_DWRR_ENABLE_PER_PRIORITY_MASK(params->dwrr_priority , node, error, fl_changed = 1;)

#define	UPDATE_PORT_DWRR_ENABLE_PER_PRIORITY_MASK(error) 	UPDATE_DWRR_ENABLE_PER_PRIORITY_MASK( dwrr_priority , port , error,  )
/**
 */
int prvCpssSchedUpdateQ(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index,
                    struct tm_queue_params *params)
{
    int rc;
    uint8_t fl_changed = 0;
    uint8_t sh_changed = 0;
    struct tm_queue *queue = NULL;

    TM_CTL(ctl, hndl);

	CONVERT_TO_PHYSICAL(Q_LEVEL,index)   /** if supported - inside macro */
	


    /* Check parameters validity */
	VALIDATE_NODE_ALLOCATED(queue,index)
	if (rc) goto out;
   
    queue = &(ctl->tm_queue_array[index]);
    
	UPDATE_QUEUE_ELIGIBLE_FUNCTION	(fl_changed = 1)

    if (params->shaping_profile_ref != TM_INVAL)
	{
        /* Check param */
		rc= prvCpssSchedShapingCheckShapingProfileValidity(hndl,params->shaping_profile_ref, Q_LEVEL);
		if (rc) goto out;

        /* Update SW DB */
		rc=prvCpssSchedShapingRemoveNodeFromShapingProfile(hndl, queue->shaping_profile_ref, Q_LEVEL, index);
		if (rc) goto out;
 		rc=prvCpssSchedShapingAddNodeToShapingProfile(hndl, params->shaping_profile_ref, Q_LEVEL, index);
		if (rc) goto out;
		
		sh_changed = 1;
    }

    if (params->quantum != TM_INVAL_16_BIT)
	{
		VALIDATE_NODE_QUANTUM_VALUE(params->quantum, TM_CONF_Q_QUANTUM_OOR)
		if (rc) goto out; 
        /* Update SW DB */
        /* Update quantum even if DWRR is disabled */
        queue->dwrr_quantum = params->quantum;
        fl_changed = 1;
    }



    /* Download to HW */
    if (fl_changed)
	{
		if (prvSchedLowLevelSetQueueQuantum(hndl, index)
		 || prvSchedLowLevelSetNodeEligPriorityFunc(hndl, Q_LEVEL, index))
		{
			rc = TM_HW_QUEUE_CONFIG_FAIL;
			goto out;
		}
	}
	if (sh_changed)
	{
        if (prvSchedLowLevelSetNodeShapingParametersToHwWithValidation(hndl, Q_LEVEL, index))
		{
            rc = TM_HW_SHAPING_PROF_FAILED;
            goto out;
        }		
	}

out:
    return rc;
}


/** 
 */
int prvCpssSchedUpdateNodeTypeA(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index,
                     struct tm_a_node_params *params, GT_BOOL quantumOnly)
{
    int rc;
    int i;
    uint8_t fl_changed = 0;
    uint8_t sh_changed = 0;
    struct tm_a_node *node = NULL;

    TM_CTL(ctl, hndl);

    CONVERT_TO_PHYSICAL(A_LEVEL,index)   /** if supported - inside macro */



    /* Check parameters validity */
    VALIDATE_NODE_ALLOCATED(a_node,index)
    if (rc) goto out;
   
    node = &(ctl->tm_a_node_array[index]);

    if(quantumOnly == GT_FALSE)
     {
            UPDATE_NODE_ELIGIBLE_FUNCTION(A_LEVEL,ctl->tm_elig_prio_a_lvl_tbl, fl_changed = 1)
   

            if (params->shaping_profile_ref != TM_INVAL)
            {
                rc= prvCpssSchedShapingCheckShapingProfileValidity(hndl,params->shaping_profile_ref, A_LEVEL);
                if (rc) goto out;

                /* Update SW DB */
                rc=prvCpssSchedShapingRemoveNodeFromShapingProfile(hndl, node->shaping_profile_ref, A_LEVEL, index);
                if (rc) goto out;
                rc=prvCpssSchedShapingAddNodeToShapingProfile(hndl, params->shaping_profile_ref, A_LEVEL, index);
                if (rc) goto out;

                sh_changed = 1;
            }

      }

    /* DWRR for A-node */
    if (params->quantum != TM_INVAL_16_BIT)
    {
        VALIDATE_NODE_QUANTUM_VALUE(params->quantum, TM_CONF_A_QUANTUM_OOR)
        if (rc) goto out; 
        /* Update SW DB */
        node->dwrr_quantum = params->quantum;
        fl_changed = TM_ENABLE;
    }

     if(quantumOnly == GT_FALSE)
      {
            /* DWRR for Queues in A-node's range */
            UPDATE_NODE_DWRR_ENABLE_PER_PRIORITY_MASK(TM_CONF_A_DWRR_PRIO_OOR )

            /* Download to HW */
            if (fl_changed)
            {
                if (prvSchedLowLevelSet_a_node_Params(hndl, index)
                 || prvSchedLowLevelSetNodeEligPriorityFunc(hndl, A_LEVEL, index))
                {
                    rc = TM_HW_A_NODE_CONFIG_FAIL;
                    goto out;
                }
            }
            if (sh_changed)
            {
                if (prvSchedLowLevelSetNodeShapingParametersToHwWithValidation(hndl, A_LEVEL, index))
                {
                    rc = TM_HW_SHAPING_PROF_FAILED;
                    goto out;
                }       
            }
      }
     else
     {
            /* Download to HW */
            if (fl_changed)
            {
                if (prvSchedLowLevelSetANodeQuantum(hndl, index)) 
                {
                    rc = TM_HW_A_NODE_CONFIG_FAIL;
                    goto out;
                }
            }
      }
out:
    return rc;
}


/**
 */
int prvCpssSchedUpdateNodeTypeB(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index,
                     struct tm_b_node_params *params)
{
    int rc;
    int i;
    uint8_t fl_changed = 0;
    uint8_t sh_changed = 0;
    struct tm_b_node *node = NULL;

    TM_CTL(ctl, hndl);

	CONVERT_TO_PHYSICAL(B_LEVEL,index)   /** if supported - inside macro */
	


    /* Check parameters validity */
	VALIDATE_NODE_ALLOCATED(b_node,index)
	if (rc) goto out;
   
    node = &(ctl->tm_b_node_array[index]);
    
	UPDATE_NODE_ELIGIBLE_FUNCTION(B_LEVEL,ctl->tm_elig_prio_b_lvl_tbl,  fl_changed = 1)

    if (params->shaping_profile_ref != TM_INVAL)
	{
 		rc= prvCpssSchedShapingCheckShapingProfileValidity(hndl,params->shaping_profile_ref, B_LEVEL);
		if (rc) goto out;

        /* Update SW DB */
 		rc=prvCpssSchedShapingRemoveNodeFromShapingProfile(hndl, node->shaping_profile_ref, B_LEVEL, index);
		if (rc) goto out;
		rc=prvCpssSchedShapingAddNodeToShapingProfile(hndl, params->shaping_profile_ref, B_LEVEL, index);
		if (rc) goto out;
		sh_changed=1;
    }

    /* DWRR for B-node */
    if (params->quantum != TM_INVAL_16_BIT)
	{
		VALIDATE_NODE_QUANTUM_VALUE(params->quantum, TM_CONF_B_QUANTUM_OOR)
		if (rc) goto out; 
        /* Update SW DB */
        node->dwrr_quantum = params->quantum;
        fl_changed = 1;
    }

    /* DWRR for A-nodes in B-node's range */
	UPDATE_NODE_DWRR_ENABLE_PER_PRIORITY_MASK(TM_CONF_B_DWRR_PRIO_OOR )
	

    
    /* Download to HW */
    if (fl_changed)
	{
		if (prvSchedLowLevelSet_b_node_Params(hndl, index)
		 || prvSchedLowLevelSetNodeEligPriorityFunc(hndl, B_LEVEL, index))
		{
			rc = TM_HW_B_NODE_CONFIG_FAIL;
			goto out;
		}
	}
	if (sh_changed)
	{
        if (prvSchedLowLevelSetNodeShapingParametersToHwWithValidation(hndl, B_LEVEL, index))
		{
            rc = TM_HW_SHAPING_PROF_FAILED;
            goto out;
        }		
	}
out:
    return rc;
}


/** 
 */
int prvCpssSchedUpdateNodeTypeC(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index,
                     struct tm_c_node_params *params)
{
    
    int rc = 0;
    int i;
    uint8_t fl_changed = 0;
    uint8_t sh_changed = 0;
	struct tm_c_node *node = NULL;

    TM_CTL(ctl, hndl);

	CONVERT_TO_PHYSICAL(Q_LEVEL,index)   /** if supported - inside macro */



    /* Check parameters validity */
	VALIDATE_NODE_ALLOCATED(c_node,index)
	if (rc) goto out;
   
    node = &(ctl->tm_c_node_array[index]);

	UPDATE_NODE_ELIGIBLE_FUNCTION(C_LEVEL,ctl->tm_elig_prio_c_lvl_tbl, fl_changed = 1)

    if (params->shaping_profile_ref != TM_INVAL)
	{
 		rc= prvCpssSchedShapingCheckShapingProfileValidity(hndl,params->shaping_profile_ref, C_LEVEL);
		if (rc) goto out;
		
        /* Update SW DB */
		rc=prvCpssSchedShapingRemoveNodeFromShapingProfile(hndl, node->shaping_profile_ref, C_LEVEL, index);
		if (rc) goto out;
		rc=prvCpssSchedShapingAddNodeToShapingProfile(hndl, params->shaping_profile_ref, C_LEVEL, index);
		if (rc) goto out;
		sh_changed=1;
    }

    /* DWRR for C-node */
    if (params->quantum != TM_INVAL_16_BIT)
	{
		VALIDATE_NODE_QUANTUM_VALUE(params->quantum, TM_CONF_C_QUANTUM_OOR)
		if (rc) goto out; 
        /* Update SW DB */
        node->dwrr_quantum = params->quantum;
        fl_changed = 1;
    }

    /* DWRR for B-nodes in C-node's range */
	UPDATE_NODE_DWRR_ENABLE_PER_PRIORITY_MASK(TM_CONF_C_DWRR_PRIO_OOR )


 
    /* Download to HW */
    if (fl_changed)
	{
 		if (prvSchedLowLevelSet_c_node_Params(hndl, index)
		 || prvSchedLowLevelSetNodeEligPriorityFunc(hndl, C_LEVEL, index))
		{
			rc = TM_HW_C_NODE_CONFIG_FAIL;
			goto out;
		}
	}
	if (sh_changed)
	{
        if (prvSchedLowLevelSetNodeShapingParametersToHwWithValidation(hndl, C_LEVEL, index))
		{
            rc = TM_HW_SHAPING_PROF_FAILED;
            goto out;
        }		
	}
out:
    return rc;
}

/**
 */
int prvCpssSchedUpdatePortShaping(PRV_CPSS_SCHED_HANDLE hndl, uint8_t index,
                           uint32_t cir_bw,
                           uint32_t eir_bw,
                           uint32_t cbs,
                           uint32_t ebs)
{
    uint32_t    port_cir_bw;
    uint32_t    port_eir_bw;
    uint32_t    port_cbs;
    uint32_t    port_ebs;
    uint16_t    min_token;
    uint16_t    max_token;
    uint8_t     res_min_bw;
    uint8_t     res_max_bw;
    uint16_t    periods;
    int         rc = 0;
    struct tm_port *port = NULL;

    TM_CTL(ctl, hndl);


    VALIDATE_NODE_ALLOCATED(port,index)
    if (rc) goto out;
   
    
    port = &(ctl->tm_port_array[index]);

    /* check shaping parameters */
    if ((cbs != TM_INVAL) && (cbs > TM_128M_kB))
    {
        rc = TM_CONF_PORT_BS_OOR;
        goto out;
    }
    if ((ebs != TM_INVAL) && (ebs > TM_128M_kB))
    {
        rc = TM_CONF_PORT_BS_OOR;
        goto out;
    }


    if (cir_bw != TM_INVAL) port_cir_bw = cir_bw;
    else                    port_cir_bw = TM_MAX_BW;
    if (eir_bw != TM_INVAL) port_eir_bw = eir_bw;
    else                    port_eir_bw = TM_MAX_BW;

#if 0
    if (cir_bw != TM_INVAL) port_cir_bw = cir_bw;
    else                    port_cir_bw = ctl->tm_port_array[index].cir_sw;
    if (eir_bw != TM_INVAL) port_eir_bw = eir_bw;
    else                    port_eir_bw = ctl->tm_port_array[index].eir_sw;

    if (cbs != TM_INVAL) port_cbs = cbs;
    else                 port_cbs = ctl->tm_port_array[index].cir_burst_size;

    if (ebs != TM_INVAL) port_ebs = ebs;
    else                 port_ebs = ctl->tm_port_array[index].eir_burst_size;
#endif

    if (port_cir_bw == TM_MAX_BW)
        port_cbs = TM_MAX_BURST;
    else
        port_cbs = cbs;

    if (port_eir_bw == TM_MAX_BW)
        port_ebs = TM_MAX_BURST;
    else
        port_ebs = ebs;

    rc = prvSchedShapingUtilsCalculatePortShaping(1000.0*port_cir_bw/ctl->level_data[P_LEVEL].unit,
                                1000.0*port_eir_bw/ctl->level_data[P_LEVEL].unit,
                                &port_cbs,
                                &port_ebs,
                                (1 << ctl->level_data[P_LEVEL].token_res_exp),
                                &periods,
                                &res_min_bw, &res_max_bw,
                                &min_token, &max_token);
    if (rc)
        goto out;

#ifdef DEBUG
PRINT_PORT_SHAPING_CALCULATION_RESULT(port_cir_bw,port_eir_bw)
#endif

    /* update shaping parameters in SW */
    port->cir_sw = port_cir_bw;
    port->eir_sw = port_eir_bw;
    port->cir_burst_size = port_cbs;
    port->eir_burst_size = port_ebs;
    port->cir_token = min_token;
    port->eir_token = max_token;
    port->periods = periods;
    port->min_token_res = res_min_bw;
    port->max_token_res = res_max_bw;

    rc = prvSchedLowLevelSetPortShapingParametersToHw(hndl, index);
    if (rc)
        rc = TM_HW_PORT_CONFIG_FAIL;
out:
      
    return rc;
}

int prvCpssSchedVerifyPortShapingConfiguration(PRV_CPSS_SCHED_HANDLE hndl,
											uint32_t cir_bw,
											uint32_t eir_bw,
											uint32_t * pcbs,
											uint32_t * pebs)
{
    uint32_t 	port_cir_bw;
    uint32_t 	port_eir_bw;
    uint16_t 	token;
    uint8_t 	res_bw;
    uint16_t 	periods;
    int 		rc = 0;

    TM_CTL(ctl, hndl);

    if (ctl->periodic_scheme_state != TM_ENABLE)
    {
        rc = TM_CONF_UPD_RATE_NOT_CONF_FOR_LEVEL;
        goto out;
    }

    /* check shaping parameters */
    if ((*pcbs > TM_128M_kB) || (*pebs > TM_128M_kB))
    {
    	rc = TM_CONF_PORT_BS_OOR;
        goto out;
    }


	if (cir_bw != TM_INVAL) port_cir_bw = cir_bw;
	else					port_cir_bw = TM_MAX_BW;
	if (eir_bw != TM_INVAL)	port_eir_bw = eir_bw;
	else					port_eir_bw = TM_MAX_BW;

    if (port_cir_bw == TM_MAX_BW)
        *pcbs = TM_MAX_BURST;

    if (port_eir_bw == TM_MAX_BW)
        *pebs = TM_MAX_BURST;

	rc = prvSchedShapingUtilsCalculatePortShaping(1000.0*(port_cir_bw)/ctl->level_data[P_LEVEL].unit,
								1000.0*(port_eir_bw)/ctl->level_data[P_LEVEL].unit,
								pcbs,
								pebs,
								(1 << ctl->level_data[P_LEVEL].token_res_exp),
								&periods,
								&res_bw, &res_bw,
								&token, &token);
out:
    return rc;
}

/**
 */
int prvCpssSchedUpdatePortScheduling
(                 PRV_CPSS_SCHED_HANDLE hndl,
                  uint8_t index,
                  uint8_t elig_prio_func_ptr,
                  uint16_t *quantum, /* 8 cells array */
                  uint8_t *dwrr_priority,
                  GT_BOOL quantumOnly)
{
    struct tm_port *port = NULL;
    int rc = 0;
    int i;

    TM_CTL(ctl, hndl);



    VALIDATE_NODE_ALLOCATED(port,index)
    if (rc) goto out;
   
    port = &(ctl->tm_port_array[index]);

    UPDATE_ELIGIBLE_FUNCTION(P_LEVEL,ctl->tm_elig_prio_p_lvl_tbl, prvSchedIsNodeEligFuncUseShaper, elig_prio_func_ptr, port->elig_prio_func, /*nothing to addition actions for port */)

    /* DWRR for Port */
    if ((quantum[0] != TM_INVAL_16_BIT) ||
        (quantum[2] != TM_INVAL_16_BIT) ||
        (quantum[3] != TM_INVAL_16_BIT) ||
        (quantum[4] != TM_INVAL_16_BIT) ||
        (quantum[5] != TM_INVAL_16_BIT) ||
        (quantum[6] != TM_INVAL_16_BIT) ||
        (quantum[7] != TM_INVAL_16_BIT))
    {
        /* Check quantum */
        for (i=0; i<8; i++)
        {
            if(quantum[i] != TM_INVAL_16_BIT)
            {
                VALIDATE_PORT_QUANTUM_VALUE(quantum[i] , TM_CONF_PORT_QUANTUM_OOR)
                if (rc) goto out; 
                   /* Update SW DB */
                port->dwrr_quantum_per_level[i].quantum = quantum[i];
            }
        }
    }

     if(quantumOnly == GT_FALSE)
     {

    /* DWRR for C-nodes in Port's range */
    UPDATE_PORT_DWRR_ENABLE_PER_PRIORITY_MASK(TM_CONF_PORT_DWRR_PRIO_OOR)
    }
     
    rc = prvSchedLowLevelSetPortScheduling(hndl, index,quantumOnly);
    
    if (rc)
    {
        rc = TM_HW_PORT_CONFIG_FAIL;
        goto out;
    }

    if(quantumOnly == GT_FALSE)
    {
        /* Download port eligible function pointer (from profile) to HW */
        rc = prvSchedLowLevelSetNodeEligPriorityFunc(hndl, P_LEVEL, index);
        if (rc) 
            rc = TM_HW_ELIG_PRIO_FUNC_FAILED;
      }

out:
      
    return rc;
}




