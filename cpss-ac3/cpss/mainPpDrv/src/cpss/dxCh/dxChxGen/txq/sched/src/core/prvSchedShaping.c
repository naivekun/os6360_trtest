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
 * @brief tm_shaping implementation.
 *
* @file tm_shaping.c
*
* $Revision: 2.0 $
 */





#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedShaping.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrCodes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedSetLocalDbDefaults.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSetHwRegisters.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedShapingUtils.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedNodesCtlInternal.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedOsInterface.h>


#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvRmAlloc.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvRmStatus.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvRmFree.h>

#include <errno.h>
#include <stdlib.h>
#include <assert.h>





/**
#define PRINT_SHAPING_RESULTS


*/


int prvCpssSchedShapingInitInfiniteShapingProfile(PRV_CPSS_SCHED_HANDLE hndl)
{
    int default_profile_index;
    TM_CTL(ctl, hndl)
    default_profile_index = prvCpssRmFindFreeShapingProfile(ctl->rm);
    /* default_profile_index must be first in resource pool , otherwise function is called by wrong way */
    if (default_profile_index != SCHED_INF_SHP_PROFILE)
	{
        return  -ENOSPC;
    }
    return prvSchedSetSwShapingProfileDefault(ctl->tm_shaping_profiles,SCHED_INF_SHP_PROFILE);
}



int prvCpssSchedShapingCreateShapingProfile(PRV_CPSS_SCHED_HANDLE hndl,
                              enum schedLevel level,
                              struct tm_shaping_profile_params *profile,
                              uint32_t * prof_index)
{
    int rc;
    struct prvCpssDxChTxqSchedShapingProfile *sh_profile;
    int prof_ind = (int)TM_INVAL;
    
    uint16_t	cir_token;
    uint16_t	eir_token;
    uint8_t		cir_resolution;
    uint8_t		eir_resolution;
    uint8_t		min_div_exp;
    uint8_t		max_div_exp;
    uint16_t	cbs;
    uint16_t	ebs;

    TM_CTL(ctl, hndl)

	if (!profile)			return -EACCES;
	if (!prof_index)		return -EACCES;
	if ((uint32_t)level > C_LEVEL)	return -EADDRNOTAVAIL;




    /* 0<cbs<=TM_4M_kB or 0<ebs<=TM_4M_kB */
    if ((profile->cbs> TM_4M_kB) || (profile->ebs > TM_4M_kB))
	{
        rc = -EFAULT;
        goto out;
    }
    
    /* check if shaping is not disabled/not yet set for the level */
    if (ctl->periodic_scheme_state != TM_ENABLE)
	{
        rc = TM_CONF_UPD_RATE_NOT_CONF_FOR_LEVEL;
        goto out;
    }

    prof_ind = prvCpssRmFindFreeShapingProfile(ctl->rm);
    if (prof_ind < 0)
	{
        rc = -ENOSPC;
        goto out;
    }
	else
	{
        *prof_index = prof_ind;
	}
	cbs = profile->cbs;
	ebs = profile->ebs;
    schedPrintf("eir_bw = %d \n",profile->eir_bw);
    
    rc = prvSchedShapingUtilsCalculateNodeShaping(1000.0*profile->cir_bw/ctl->level_data[level].unit,
								1000.0*profile->eir_bw/ctl->level_data[level].unit,
								&cbs,
								&ebs,
								(1 << ctl->level_data[level].token_res_exp),
								&min_div_exp,
								&max_div_exp,
								&cir_resolution,
								&eir_resolution,
								&cir_token, &eir_token,hndl,NULL);
    if (rc) goto out;

#ifdef DEBUG
    PRINT_NODE_SHAPING_CALCULATION_RESULT(profile->cir_bw,profile->eir_bw);
#endif

    /* update the SW image */
    sh_profile = &(ctl->tm_shaping_profiles[prof_ind]);
    sh_profile->use_counter = 0;
	sh_profile->level=level;  
	sh_profile->cir_bw=profile->cir_bw;
	sh_profile->eir_bw=profile->eir_bw;
    sh_profile->min_token = (uint16_t)cir_token;
    sh_profile->min_burst_size = (uint16_t)profile->cbs;
    sh_profile->max_token = (uint16_t)eir_token;
    sh_profile->max_burst_size = (uint16_t)profile->ebs;
	sh_profile->min_div_exp = min_div_exp;
	sh_profile->max_div_exp = max_div_exp;
    sh_profile->min_token_res = cir_resolution;
    sh_profile->max_token_res = eir_resolution;
    
out:
    if (rc)
	{
        /* free shaping profile */
        if (prof_ind != (int)TM_INVAL)
            prvSchedRmFreeShapingProfile(ctl->rm, prof_ind);
    }
    return rc;
}




/* MACRO for update all layer nodes referenced to updated shaping profile */

#define	UPDATE_PROFILE_CLIENTS(layer, layer_size) \
	i=0;\
	node_counter=0;\
	while ( (node_counter < sh_profile->use_counter) && (i < layer_size)) \
	{\
		if (prof_index==ctl->layer[i].shaping_profile_ref) \
		{\
			/* update node */\
			if (prvSchedLowLevelSetNodeShapingParametersToHwWithValidation(hndl, level, i) < 0)\
			{\
				rc = TM_HW_SHAPING_PROF_FAILED;\
				goto out;\
			}\
			node_counter++;\
		}\
		i++;\
	}\
/* fprintf(stderr,">>>>>>>>>>>>>>>>>>>>>>>>>>> %s        level=%d,  use_counter=%d ,  node_counter=%d,  index=%d\n",__FUNCTION__, level, sh_profile->use_counter , node_counter, i) 	; */




/**
 */
int prvCpssSchedShapingUpdateShapingProfile(PRV_CPSS_SCHED_HANDLE hndl,
                              enum schedLevel level,
                              uint32_t prof_index,
                              struct tm_shaping_profile_params *profile)
{
   struct prvCpssDxChTxqSchedShapingProfile *sh_profile;

	int			rc;

    uint16_t	cir_token;
    uint16_t	eir_token;
    uint8_t	cir_resolution;
    uint8_t	eir_resolution;
    uint8_t	min_div_exp;
    uint8_t	max_div_exp;

    uint32_t 	min_bw;
    uint32_t 	max_bw;
    
	uint16_t 	min_burst_size;
    uint16_t 	max_burst_size;

    uint32_t 	node_counter;
    uint32_t  	i;

    TM_CTL(ctl, hndl)




    if (prof_index == SCHED_INF_SHP_PROFILE) { /* reserved profile */
        rc = -EACCES;
        goto out;
    }
    
	/* validate shaping profile */
 	TEST_RESOURCE_EXISTS(prvSchedRmShapingProfileStatus(ctl->rm,prof_index) ,/* out_of range error*/  -EACCES, /*profile not used error*/ -EPERM)
 	if (rc) goto out;

    sh_profile = &(ctl->tm_shaping_profiles[prof_index]);
	/*check if profile index match level*/
	if(sh_profile->level!=level)
	{
		rc = -EADDRNOTAVAIL;
		goto out;
	}
   
    if ((profile->cbs != TM_INVAL_16_BIT) && (profile->cbs > TM_4M_kB))
	{
        rc = -EFAULT;
        goto out;
    }
	if ((profile->ebs != TM_INVAL_16_BIT) && (profile->ebs > TM_4M_kB))
	{
		rc = -EFAULT;
		goto out;
	} 

    if (profile->cir_bw != TM_INVAL)	min_bw = profile->cir_bw;
    else								min_bw = sh_profile->cir_bw;
    
    if (profile->eir_bw != TM_INVAL)    max_bw = profile->eir_bw;
    else								max_bw = sh_profile->eir_bw;

	if (profile->cbs != TM_INVAL_16_BIT)		min_burst_size = profile->cbs;
	else								min_burst_size = sh_profile->min_burst_size;

	if (profile->ebs != TM_INVAL_16_BIT)		max_burst_size = profile->ebs;
	else								max_burst_size = sh_profile->max_burst_size;

    rc = prvSchedShapingUtilsCalculateNodeShaping(1000.0 * min_bw/ctl->level_data[level].unit, 
								1000.0 * max_bw/ctl->level_data[level].unit, 
								&min_burst_size,
								&max_burst_size,
								(1 << ctl->level_data[level].token_res_exp),
								&min_div_exp,
								&max_div_exp,
								&cir_resolution,
								&eir_resolution,
								&cir_token,
								&eir_token,hndl,NULL);
    if (rc)
        goto out;

#ifdef DEBUG
    PRINT_NODE_SHAPING_CALCULATION_RESULT(min_bw , max_bw);
#endif

    /* update the SW image */
	sh_profile->cir_bw=min_bw;
	sh_profile->eir_bw=max_bw;
    sh_profile->min_token = cir_token;
    sh_profile->min_burst_size = min_burst_size;
    sh_profile->max_token = eir_token;
    sh_profile->max_burst_size = max_burst_size;
    sh_profile->min_div_exp = min_div_exp;
    sh_profile->max_div_exp = max_div_exp;
    sh_profile->min_token_res = cir_resolution;
    sh_profile->max_token_res = eir_resolution;
     
    /* update shaping for all profile clients */
    switch(level)
    {
		case Q_LEVEL:
		{
                  UPDATE_PROFILE_CLIENTS(tm_queue_array,ctl->tm_total_queues);
			break;
		}
		case A_LEVEL:
		{
                  UPDATE_PROFILE_CLIENTS(tm_a_node_array,ctl->tm_total_a_nodes);
			break;
		}
		case B_LEVEL:
		{
                  UPDATE_PROFILE_CLIENTS(tm_b_node_array,ctl->tm_total_b_nodes);
			break;
		}
		case C_LEVEL:
		{
                  UPDATE_PROFILE_CLIENTS(tm_c_node_array,ctl->tm_total_c_nodes);
			break;
		}
		default: break;
	}	
out:
    
    return rc;
}


#define __RETURN_PROF_REF_OOR_ERROR_CODE \
{\
	switch (level)\
	{\
			case Q_LEVEL: return TM_CONF_Q_SHAPING_PROF_REF_OOR; \
			case A_LEVEL: return TM_CONF_A_SHAPING_PROF_REF_OOR;\
			case B_LEVEL: return TM_CONF_B_SHAPING_PROF_REF_OOR;\
			case C_LEVEL: return TM_CONF_C_SHAPING_PROF_REF_OOR;\
			default:  return TM_CONF_SHAPING_PROF_REF_OOR; /* not happend */\
	}\
}

int prvCpssSchedShapingCheckShapingProfileValidity(	PRV_CPSS_SCHED_HANDLE hndl, uint32_t prof_index, enum schedLevel level)
{
    TM_CTL(ctl, hndl);

#ifdef PER_NODE_SHAPING
	/* direct shaping can't be input parameter - so profile is invalid for  create/update purposes */
    if (prof_index ==SCHED_DIRECT_NODE_SHAPING) __RETURN_PROF_REF_OOR_ERROR_CODE;
#endif
    if (prvSchedRmShapingProfileStatus(ctl->rm, prof_index) != 1 )  __RETURN_PROF_REF_OOR_ERROR_CODE;
	if ((prof_index) && (ctl->tm_shaping_profiles[prof_index].level != level)) return TM_WRONG_SHP_PROFILE_LEVEL;
	return 0;
}


#define SET_SHAPING_PROFILE_TO_NODE(profile) \
    switch(level) \
    {\
		case Q_LEVEL:ctl->tm_queue_array[node_index].shaping_profile_ref=profile; break;\
		case A_LEVEL:ctl->tm_a_node_array[node_index].shaping_profile_ref=profile; break;\
		case B_LEVEL:ctl->tm_b_node_array[node_index].shaping_profile_ref=profile; break;\
		case C_LEVEL:ctl->tm_c_node_array[node_index].shaping_profile_ref=profile; break;\
		default: break;\
	}

#define GET_NODE_SHAPING_PROFILE(profile) \
    switch(level) \
    {\
		case Q_LEVEL:profile = ctl->tm_queue_array[node_index].shaping_profile_ref; break;\
		case A_LEVEL:profile = ctl->tm_a_node_array[node_index].shaping_profile_ref; break;\
		case B_LEVEL:profile = ctl->tm_b_node_array[node_index].shaping_profile_ref; break;\
		case C_LEVEL:profile = ctl->tm_c_node_array[node_index].shaping_profile_ref; break;\
		default:assert(0);profile=0xFFFF; break;				\
	}





int prvCpssSchedShapingAddNodeToShapingProfile(PRV_CPSS_SCHED_HANDLE hndl,
								uint32_t prof_index,
								enum schedLevel level,
								uint32_t node_index)
{
    int rc;
    TM_CTL(ctl, hndl);
	(void)node_index;
#ifdef PER_NODE_SHAPING
	/* this can't happend  */
	if (prof_index==SCHED_DIRECT_NODE_SHAPING)  return 0; /* direct shaping  -  node is not client  */
#endif 
	if (prof_index==SCHED_INF_SHP_PROFILE)  return 0; /* default profile not updated, so it doesn't need clients */
	rc = prvCpssSchedShapingCheckShapingProfileValidity(hndl,prof_index, level);
	if (rc) return rc;
	SET_SHAPING_PROFILE_TO_NODE((uint16_t)prof_index);
	ctl->tm_shaping_profiles[prof_index].use_counter++;	
	return 0;
}

int prvCpssSchedShapingRemoveNodeFromShapingProfile(PRV_CPSS_SCHED_HANDLE hndl,
								uint32_t prof_index,
								enum schedLevel level,
								uint32_t node_index)
{
    int rc;
    TM_CTL(ctl, hndl);
	(void)node_index;
#ifdef PER_NODE_SHAPING
	if (prof_index==SCHED_DIRECT_NODE_SHAPING)  return 0; /* direct shaping  -  node is not client  */
#endif 
	if (prof_index==SCHED_INF_SHP_PROFILE)  return 0; /* default profile not updated, so it doesn't need clients */
	rc = prvCpssSchedShapingCheckShapingProfileValidity(hndl,prof_index, level);
	if (rc) return rc;
	/* reset node shaping profile ref  to  default (unupdated) profile*/  
	SET_SHAPING_PROFILE_TO_NODE(SCHED_INF_SHP_PROFILE);
	ctl->tm_shaping_profiles[prof_index].use_counter--;	
	return rc;
}



#ifdef PER_NODE_SHAPING

	#define COPY_NODE_SHAPING_PARAMS_TO_PTR(source_node, target_ptr) \
		target_ptr->cir_bw=source_node.cir_bw;\
		target_ptr->eir_bw=source_node.eir_bw;\
		target_ptr->cbs=source_node.cbs;\
		target_ptr->ebs=source_node.ebs;

	#define COPY_NODE_SHAPING_PARAMS_FROM_PTR(source_ptr, target_node) \
		target_node.cir_bw=source_ptr->cir_bw;\
		target_node.eir_bw=source_ptr->eir_bw;\
		target_node.cbs=source_ptr->cbs;\
		target_node.ebs=source_ptr->ebs;


    int prvCpssSchedShapingUpdateNodeShapingProc
  ( 
        PRV_CPSS_SCHED_HANDLE hndl,
        enum schedLevel level,
        uint32_t node_index,
        struct tm_shaping_profile_params * node_shaping_params, 
        uint32_t * actual_cir_bw_received_ptr
  )
    {
        int rc;
        uint16_t    node_shaping_profile;
        
        uint16_t    cir_token;
        uint16_t    eir_token;
        uint8_t cir_resolution;
        uint8_t eir_resolution;
        uint8_t min_div_exp;
        uint8_t max_div_exp;
        struct  prvCpssDxChTxqSchedShapingProfile tmp_shp_profile;
        uint16_t        cbs;
        uint16_t        ebs;
        double      accuracy;
        uint32_t    providedBw = node_shaping_params->cir_bw;

        TM_CTL(ctl, hndl)


        /* Check node  validity */
        switch (level)
        {
            case Q_LEVEL:
                VALIDATE_NODE_ALLOCATED(queue,node_index)
                break;
            case A_LEVEL:
                VALIDATE_NODE_ALLOCATED(a_node,node_index)
                break;
            case B_LEVEL:
                VALIDATE_NODE_ALLOCATED(b_node,node_index)
                break;
            case C_LEVEL:
                VALIDATE_NODE_ALLOCATED(c_node,node_index)
               break;
            case P_LEVEL:
                VALIDATE_NODE_ALLOCATED(port,node_index)
                break;
            default:
                rc = -ERANGE;
        }
        if (rc) goto out;
        
        /* Check shaping params validity */
        if ((node_shaping_params->cbs > TM_4M_kB) || (node_shaping_params->ebs > TM_4M_kB))
        {
            rc = -EFAULT;
            goto out;
        }
        /* calculate shaping  */
        cbs = node_shaping_params->cbs;
        ebs = node_shaping_params->ebs;

        rc = prvSchedShapingUtilsCalculateNodeShaping(1000.0 * node_shaping_params->cir_bw/ctl->level_data[level].unit, 
                                    1000.0 * node_shaping_params->eir_bw/ctl->level_data[level].unit, 
                                    &cbs,
                                    &ebs,
                                    (1 << ctl->level_data[level].token_res_exp),
                                    &min_div_exp,
                                    &max_div_exp,
                                    &cir_resolution,
                                    &eir_resolution,
                                    &cir_token,
                                    &eir_token,hndl,&accuracy);
        if (rc)
            goto out;

        PRINT_NODE_SHAPING_CALCULATION_RESULT(node_shaping_params->cir_bw ,node_shaping_params->eir_bw)
        /* if valid : */
        GET_NODE_SHAPING_PROFILE(node_shaping_profile);
        rc=prvCpssSchedShapingRemoveNodeFromShapingProfile(hndl, node_shaping_profile, level, node_index);
        if (rc) goto out;
        tmp_shp_profile.level=level;  
        tmp_shp_profile.cir_bw=node_shaping_params->cir_bw;
        tmp_shp_profile.eir_bw=node_shaping_params->eir_bw;
        tmp_shp_profile.min_token = cir_token;
        tmp_shp_profile.min_burst_size = node_shaping_params->cbs;
        tmp_shp_profile.max_token = 0xFFF;/*no use in max token in Falcon*/
        tmp_shp_profile.max_burst_size = 0xFFF;/*no use in max token in Falcon*/
        tmp_shp_profile.min_div_exp = min_div_exp;
        tmp_shp_profile.max_div_exp = max_div_exp;
        tmp_shp_profile.min_token_res = cir_resolution;
        tmp_shp_profile.max_token_res = eir_resolution;   

        /*Calculate actual bandwith*/
      
          providedBw += (uint32_t)(accuracy*providedBw);
          *actual_cir_bw_received_ptr = providedBw;
        
#ifdef PRINT_SHAPING_RESULTS
        schedPrintf("\nactual_cir_bw_received :%u  accuracy %lf \n", *actual_cir_bw_received_ptr ,accuracy);
#endif        
        rc = prvSchedLowLevelSetNodeShapingParametersToHw(hndl, level, node_index, &tmp_shp_profile);
        /* Update SW DB */
        if (rc)
        {
            rc = TM_HW_SHAPING_PROF_FAILED;
            /* rollback  -return node to clients set   ??? */
            prvCpssSchedShapingAddNodeToShapingProfile(hndl, node_shaping_profile, level, node_index);
            goto out;
        }
        SET_SHAPING_PROFILE_TO_NODE(SCHED_DIRECT_NODE_SHAPING);
        /* store  node shaping parameters */  
        switch(level) 
        {
            case Q_LEVEL:COPY_NODE_SHAPING_PARAMS_FROM_PTR(node_shaping_params ,ctl->tm_queue_array[node_index] ); break;
            case A_LEVEL:COPY_NODE_SHAPING_PARAMS_FROM_PTR(node_shaping_params ,ctl->tm_a_node_array[node_index]); break;
            case B_LEVEL:COPY_NODE_SHAPING_PARAMS_FROM_PTR(node_shaping_params ,ctl->tm_b_node_array[node_index]); break;
            case C_LEVEL:COPY_NODE_SHAPING_PARAMS_FROM_PTR(node_shaping_params ,ctl->tm_c_node_array[node_index]); break;
            default: break;
        }
    out:
        return rc;
    }


	int prvCpssSchedShapingUpdateNodeShapingProRreadNodeShapingProc(PRV_CPSS_SCHED_HANDLE hndl,
								  enum schedLevel level,
								  uint32_t node_index,
								  uint16_t * shaping_profile,
								  struct tm_shaping_profile_params * node_shaping_params)
	{
		int rc;

		TM_CTL(ctl, hndl);
		


		/* Check node  validity */
		switch (level)
		{
		case Q_LEVEL:
			if (node_index >= ctl->tm_total_queues) rc = -EFAULT;
			else if (ctl->tm_queue_array[node_index].mapping.nodeStatus != TM_NODE_USED)  rc = -ENODATA;
			else rc = 0;
			break;
		case A_LEVEL:
			if (node_index >= ctl->tm_total_a_nodes) rc = -EFAULT;
			else if (ctl->tm_a_node_array[node_index].mapping.nodeStatus != TM_NODE_USED)  rc = -ENODATA;
			else rc = 0;
			break;
		case B_LEVEL:
			if (node_index >= ctl->tm_total_b_nodes) rc = -EFAULT;
			else if (ctl->tm_b_node_array[node_index].mapping.nodeStatus != TM_NODE_USED)  rc = -ENODATA;
			else rc = 0;
			break;
		case C_LEVEL:
			if (node_index >= ctl->tm_total_c_nodes) rc = -EFAULT;
			else if (ctl->tm_c_node_array[node_index].mapping.nodeStatus != TM_NODE_USED)  rc = -ENODATA;
			else rc = 0;
		   break;
		default:
			rc = -ERANGE;
		}
		if (rc) goto out;

		GET_NODE_SHAPING_PROFILE(* shaping_profile);
		
		if (* shaping_profile == SCHED_DIRECT_NODE_SHAPING)
		{
			/* fill  structure by shaping parameters */
			switch(level) 
			{
				case Q_LEVEL:COPY_NODE_SHAPING_PARAMS_TO_PTR(ctl->tm_queue_array[node_index] ,node_shaping_params); break;
				case A_LEVEL:COPY_NODE_SHAPING_PARAMS_TO_PTR(ctl->tm_a_node_array[node_index],node_shaping_params); break;
				case B_LEVEL:COPY_NODE_SHAPING_PARAMS_TO_PTR(ctl->tm_b_node_array[node_index],node_shaping_params); break;
				case C_LEVEL:COPY_NODE_SHAPING_PARAMS_TO_PTR(ctl->tm_c_node_array[node_index],node_shaping_params); break;
				default: break;
			}
		}

	out:
		return rc;
	}

 int prvCpssSchedShapingUpdateNodeShaping
  ( 
         PRV_CPSS_SCHED_HANDLE hndl,
         enum schedLevel level,
         uint32_t node_index,
         struct tm_shaping_profile_params * node_shaping_params,
         uint32_t * actual_cir_bw_received_ptr
   )
    {   
        
        DECLARE_TM_CTL_PTR(ctl, hndl);
                  
        (void)ctl; /* prevent warning if virtual layer is not used */
        CONVERT_TO_PHYSICAL(level,node_index);
        return prvCpssSchedShapingUpdateNodeShapingProc( hndl, level, node_index, node_shaping_params,actual_cir_bw_received_ptr);
    }


	int prvCpssSchedShapingReadNodeShaping(PRV_CPSS_SCHED_HANDLE hndl,
								  enum schedLevel level,
								  uint32_t node_index,
								  uint16_t * shaping_profile,
								  struct tm_shaping_profile_params * node_shaping_params)
	{
		DECLARE_TM_CTL_PTR(ctl, hndl);
		(void)ctl;/* prevent warning if virtual layer is not used */
		CONVERT_TO_PHYSICAL(level,node_index);
		return prvCpssSchedShapingUpdateNodeShapingProRreadNodeShapingProc( hndl, level, node_index, shaping_profile, node_shaping_params);
	}


#endif /* PER_NODE_SHAPING */
