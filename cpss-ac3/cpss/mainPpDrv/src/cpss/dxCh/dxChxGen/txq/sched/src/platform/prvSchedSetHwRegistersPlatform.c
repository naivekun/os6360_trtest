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
 * @brief  functions for set/get  platform specific registers
 *
* @file set_hw_registers_platform.c
*
* $Revision: 2.0 $
 */
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedOsInterface.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedCoreTypes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedRegistersInterface.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedSetHwRegistersImp.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSetHwRegisters.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedSetLocalDbDefaults.h>





#define PRV_CPSS_DXCH_SIP6_TXQ_ON_EMULATOR           cpssDeviceRunCheck_onEmulator



int prvSchedLowLevelSetPerLevelShapingParameters(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel level)
{
    int rc = -ERANGE;
    
   

     
#if 0
    TM_REGISTER_VAR(TM_Sched_PPerCtlConf_Addr)
    TM_REGISTER_VAR(TM_Sched_CPerCtlConf_Addr)
    TM_REGISTER_VAR(TM_Sched_BPerCtlConf_Addr)
#endif

    TM_REGISTER_VAR(TM_Sched_APerCtlConf_Addr)
    TM_REGISTER_VAR(TM_Sched_QPerCtlConf_Addr)

    TM_CTL(ctl, hndl)

    if(level >=B_LEVEL&&level <=P_LEVEL)
    {   
         /*No need to configure shaping on those levels on Falcon.Only Q and A levels*/    
        rc = 0;
    }

    switch (level)
    {
    case Q_LEVEL:
        /* reset register reserved fields */
        TXQ_SCHED_RESET_REGISTER(addressSpace.Sched.QPerCtlConf_Addr, TM_Sched_QPerCtlConf_Addr)
        if (rc) break;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_QPerCtlConf_Addr, DecEn,           ctl->level_data[Q_LEVEL].shaper_dec)
        TXQ_SCHED_REGISTER_SET(TM_Sched_QPerCtlConf_Addr, PerInterval ,    ctl->level_data[Q_LEVEL].per_interval)
        TXQ_SCHED_REGISTER_SET(TM_Sched_QPerCtlConf_Addr, PerEn ,          ctl->level_data[Q_LEVEL].shaping_status)
     
           
        /* write register */
        TXQ_SCHED_WRITE_REGISTER(addressSpace.Sched.QPerCtlConf_Addr, TM_Sched_QPerCtlConf_Addr)
        break;
    case A_LEVEL:
        /* reset register reserved fields */
        TXQ_SCHED_RESET_REGISTER(addressSpace.Sched.APerCtlConf_Addr, TM_Sched_APerCtlConf_Addr)
        if (rc) break;
        /* assign register fields */        
        TXQ_SCHED_REGISTER_SET(TM_Sched_APerCtlConf_Addr, DecEn ,       ctl->level_data[A_LEVEL].shaper_dec)
        TXQ_SCHED_REGISTER_SET(TM_Sched_APerCtlConf_Addr, PerInterval , ctl->level_data[A_LEVEL].per_interval)
        TXQ_SCHED_REGISTER_SET(TM_Sched_APerCtlConf_Addr, PerEn ,       ctl->level_data[A_LEVEL].shaping_status)
      
        

        /* write register */
        TXQ_SCHED_WRITE_REGISTER(addressSpace.Sched.APerCtlConf_Addr, TM_Sched_APerCtlConf_Addr)
        break;
 #if 0   /*No need to configure shaping on those levels on Falcon*/    
    case B_LEVEL:
        /* reset register reserved fields */
        TXQ_SCHED_RESET_REGISTER(addressSpace.Sched.BPerCtlConf_Addr, TM_Sched_BPerCtlConf_Addr)
        if (rc) break;
        /* assign register fields */        
        TXQ_SCHED_REGISTER_SET(TM_Sched_BPerCtlConf_Addr, DecEn ,       ctl->level_data[B_LEVEL].shaper_dec)
        TXQ_SCHED_REGISTER_SET(TM_Sched_BPerCtlConf_Addr, PerInterval , ctl->level_data[B_LEVEL].per_interval)
        TXQ_SCHED_REGISTER_SET(TM_Sched_BPerCtlConf_Addr, PerEn ,       ctl->level_data[B_LEVEL].shaping_status)
        
        
        
        /* write register */
        TXQ_SCHED_WRITE_REGISTER(addressSpace.Sched.BPerCtlConf_Addr, TM_Sched_BPerCtlConf_Addr)
        break;
    case C_LEVEL:
        /* reset register reserved fields */
        TXQ_SCHED_RESET_REGISTER(addressSpace.Sched.CPerCtlConf_Addr, TM_Sched_CPerCtlConf_Addr)
        /* assign register fields */        
        TXQ_SCHED_REGISTER_SET(TM_Sched_CPerCtlConf_Addr, DecEn ,       ctl->level_data[C_LEVEL].shaper_dec)
        TXQ_SCHED_REGISTER_SET(TM_Sched_CPerCtlConf_Addr, PerInterval , ctl->level_data[C_LEVEL].per_interval)
        TXQ_SCHED_REGISTER_SET(TM_Sched_CPerCtlConf_Addr ,PerEn ,       ctl->level_data[C_LEVEL].shaping_status)
      
              
        /* write register */
        TXQ_SCHED_WRITE_REGISTER(addressSpace.Sched.CPerCtlConf_Addr, TM_Sched_CPerCtlConf_Addr)
        break;
    case P_LEVEL:
        /* reset register reserved fields */
        TXQ_SCHED_RESET_REGISTER(addressSpace.Sched.PPerCtlConf_Addr, TM_Sched_PPerCtlConf_Addr)
        if (rc) break;
        /* assign register fields */        
        TXQ_SCHED_REGISTER_SET(TM_Sched_PPerCtlConf_Addr, DecEn ,       ctl->level_data[P_LEVEL].shaper_dec)
        TXQ_SCHED_REGISTER_SET(TM_Sched_PPerCtlConf_Addr, PerInterval , ctl->level_data[P_LEVEL].per_interval)
        TXQ_SCHED_REGISTER_SET(TM_Sched_PPerCtlConf_Addr, PerEn ,       ctl->level_data[P_LEVEL].shaping_status);
       
        /* write register */
        TXQ_SCHED_WRITE_REGISTER(addressSpace.Sched.PPerCtlConf_Addr, TM_Sched_PPerCtlConf_Addr)
        break;
  #endif      
    default:
        break;
    }
    return rc;
}


int prvSchedLowLevelGetPerLevelShapingStatusFromHw
(   
    PRV_CPSS_SCHED_HANDLE hndl, 
    enum schedLevel level , 
    struct schedPerLevelPeriodicParams *periodic_params
)
{
    int rc = -ERANGE;

    TM_REGISTER_VAR(TM_Sched_PPerCtlConf_Addr)
    TM_REGISTER_VAR(TM_Sched_CPerCtlConf_Addr)
    TM_REGISTER_VAR(TM_Sched_BPerCtlConf_Addr)
    TM_REGISTER_VAR(TM_Sched_APerCtlConf_Addr)
    TM_REGISTER_VAR(TM_Sched_QPerCtlConf_Addr)

    TM_CTL(ctl, hndl)
    

    switch(level)
    {
    case Q_LEVEL:
        TXQ_SCHED_READ_REGISTER(addressSpace.Sched.QPerCtlConf_Addr, TM_Sched_QPerCtlConf_Addr)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_GET(TM_Sched_QPerCtlConf_Addr, DecEn,           periodic_params->shaper_dec , (uint8_t))
        TXQ_SCHED_REGISTER_GET(TM_Sched_QPerCtlConf_Addr, PerEn ,          periodic_params->per_state , (uint8_t))
        TXQ_SCHED_REGISTER_GET(TM_Sched_QPerCtlConf_Addr, PerInterval ,          periodic_params->per_interval , (uint32_t))
        break;
    case A_LEVEL:
        TXQ_SCHED_READ_REGISTER(addressSpace.Sched.APerCtlConf_Addr, TM_Sched_APerCtlConf_Addr)
        if (rc) goto out;
        /* assign register fields */        
        TXQ_SCHED_REGISTER_GET(TM_Sched_APerCtlConf_Addr, DecEn ,       periodic_params->shaper_dec , (uint8_t))
        TXQ_SCHED_REGISTER_GET(TM_Sched_APerCtlConf_Addr, PerEn ,       periodic_params->per_state , (uint8_t))
         TXQ_SCHED_REGISTER_GET(TM_Sched_APerCtlConf_Addr, PerInterval ,          periodic_params->per_interval , (uint32_t))
        break;
    case B_LEVEL:
        TXQ_SCHED_READ_REGISTER(addressSpace.Sched.BPerCtlConf_Addr, TM_Sched_BPerCtlConf_Addr)
        if (rc) goto out;
        /* assign register fields */        
        TXQ_SCHED_REGISTER_GET(TM_Sched_BPerCtlConf_Addr, DecEn ,       periodic_params->shaper_dec , (uint8_t))
        TXQ_SCHED_REGISTER_GET(TM_Sched_BPerCtlConf_Addr, PerEn ,       periodic_params->per_state , (uint8_t))
        break;
    case C_LEVEL:
        TXQ_SCHED_READ_REGISTER(addressSpace.Sched.CPerCtlConf_Addr, TM_Sched_CPerCtlConf_Addr)
        if (rc) goto out;
        /* assign register fields */        
        TXQ_SCHED_REGISTER_GET(TM_Sched_CPerCtlConf_Addr, DecEn ,       periodic_params->shaper_dec , (uint8_t))
        TXQ_SCHED_REGISTER_GET(TM_Sched_CPerCtlConf_Addr ,PerEn ,       periodic_params->per_state , (uint8_t))
        break;
    case P_LEVEL:
        TXQ_SCHED_READ_REGISTER(addressSpace.Sched.PPerCtlConf_Addr, TM_Sched_PPerCtlConf_Addr)
        if (rc) goto out;
        /* assign register fields */        
        TXQ_SCHED_REGISTER_GET(TM_Sched_PPerCtlConf_Addr, DecEn ,       periodic_params->shaper_dec , (uint8_t))
        TXQ_SCHED_REGISTER_GET(TM_Sched_PPerCtlConf_Addr, PerEn ,       periodic_params->per_state , (uint8_t))
    }
out:
    return rc;
}




/* assume here that shaping profile is valid */
int prvSchedLowLevelSetNodeShapingParametersToHw(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel level, uint32_t node_ind, struct prvCpssDxChTxqSchedShapingProfile *profile)
{
    int rc = 0;

#if 0    
 
    TM_REGISTER_VAR(TM_Sched_ClvlTokenBucketTokenEnDiv)
    TM_REGISTER_VAR(TM_Sched_BlvlTokenBucketTokenEnDiv)   
    TM_REGISTER_VAR(TM_Sched_ClvlTokenBucketBurstSize)
    TM_REGISTER_VAR(TM_Sched_BlvlTokenBucketBurstSize)
#endif    
    TM_REGISTER_VAR(TM_Sched_AlvlTokenBucketBurstSize)
    TM_REGISTER_VAR(TM_Sched_QueueTokenBucketBurstSize)

    TM_REGISTER_VAR(TM_Sched_AlvlTokenBucketTokenEnDiv)
    TM_REGISTER_VAR(TM_Sched_QueueTokenBucketTokenEnDiv)
 
    TM_CTL(ctl, hndl)

    
    if(level >=B_LEVEL&&level <=P_LEVEL)
       {   
            /*No need to configure shaping on those levels on Falcon.Only Q and A levels*/    
           return 0;
       }
    
    
    /*  profile <--> level conformance test */
    if ((profile->level !=level) && (profile->level != ALL_LEVELS))
    {
        rc = -EFAULT;
        goto out;
    }

    switch (level)
    {
    case Q_LEVEL:
        NODE_VALIDATION(ctl->tm_total_queues)
        if (rc) goto out;
        /* profile assignment */
        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.QueueTokenBucketTokenEnDiv, node_ind, TM_Sched_QueueTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* assign register fields */        
        TXQ_SCHED_REGISTER_SET(TM_Sched_QueueTokenBucketTokenEnDiv , MinDivExp ,   profile->min_div_exp);
        TXQ_SCHED_REGISTER_SET(TM_Sched_QueueTokenBucketTokenEnDiv , MaxDivExp ,   profile->max_div_exp);
                  
        TXQ_SCHED_REGISTER_SET(TM_Sched_QueueTokenBucketTokenEnDiv , MinToken ,   profile->min_token )
        TXQ_SCHED_REGISTER_SET(TM_Sched_QueueTokenBucketTokenEnDiv , MaxToken ,   profile->max_token )
        TXQ_SCHED_REGISTER_SET(TM_Sched_QueueTokenBucketTokenEnDiv , MinTokenRes ,profile->min_token_res*ctl->level_data[Q_LEVEL].token_res_exp)
        TXQ_SCHED_REGISTER_SET(TM_Sched_QueueTokenBucketTokenEnDiv , MaxTokenRes ,profile->max_token_res*ctl->level_data[Q_LEVEL].token_res_exp) 
          
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.QueueTokenBucketTokenEnDiv, node_ind, TM_Sched_QueueTokenBucketTokenEnDiv)
        if (rc) goto out;

        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.QueueTokenBucketBurstSize, node_ind,  TM_Sched_QueueTokenBucketBurstSize)
        if (rc) goto out;
        /* assign register fields */        
        TXQ_SCHED_REGISTER_SET(TM_Sched_QueueTokenBucketBurstSize , MaxBurstSz , profile->max_burst_size);
        TXQ_SCHED_REGISTER_SET(TM_Sched_QueueTokenBucketBurstSize , MinBurstSz , profile->min_burst_size);
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.QueueTokenBucketBurstSize, node_ind,  TM_Sched_QueueTokenBucketBurstSize)
        if (rc) goto out;
        break;
        
    case A_LEVEL:
        NODE_VALIDATION(ctl->tm_total_a_nodes)
        if (rc) goto out;
        /* profile assignment */
        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.AlvlTokenBucketTokenEnDiv, node_ind, TM_Sched_AlvlTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* assign register fields */        
        TXQ_SCHED_REGISTER_SET(TM_Sched_AlvlTokenBucketTokenEnDiv , MinDivExp ,    profile->min_div_exp)
        TXQ_SCHED_REGISTER_SET(TM_Sched_AlvlTokenBucketTokenEnDiv , MaxDivExp ,    profile->max_div_exp)
        

        TXQ_SCHED_REGISTER_SET(TM_Sched_AlvlTokenBucketTokenEnDiv , MinToken ,  profile->min_token )
        TXQ_SCHED_REGISTER_SET(TM_Sched_AlvlTokenBucketTokenEnDiv , MaxToken ,     profile->max_token )
        TXQ_SCHED_REGISTER_SET(TM_Sched_AlvlTokenBucketTokenEnDiv , MinTokenRes ,   profile->min_token_res*ctl->level_data[A_LEVEL].token_res_exp)
        TXQ_SCHED_REGISTER_SET(TM_Sched_AlvlTokenBucketTokenEnDiv , MaxTokenRes ,   profile->max_token_res*ctl->level_data[A_LEVEL].token_res_exp) 
        


        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.AlvlTokenBucketTokenEnDiv, node_ind, TM_Sched_AlvlTokenBucketTokenEnDiv)
        if (rc) goto out;

        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.AlvlTokenBucketBurstSize, node_ind, TM_Sched_AlvlTokenBucketBurstSize)
        if (rc) goto out;
        /* assign register fields */        
        TXQ_SCHED_REGISTER_SET(TM_Sched_AlvlTokenBucketBurstSize , MaxBurstSz , profile->max_burst_size)
        TXQ_SCHED_REGISTER_SET(TM_Sched_AlvlTokenBucketBurstSize , MinBurstSz , profile->min_burst_size)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.AlvlTokenBucketBurstSize, node_ind, TM_Sched_AlvlTokenBucketBurstSize)
        if (rc) goto out;
        break;
 #if 0       
    case B_LEVEL:
        NODE_VALIDATION(ctl->tm_total_b_nodes)
        if (rc) goto out;
        /* profile assignment */
        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.BlvlTokenBucketTokenEnDiv, node_ind, TM_Sched_BlvlTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* assign register fields */        
        TXQ_SCHED_REGISTER_SET(TM_Sched_BlvlTokenBucketTokenEnDiv , MinDivExp ,    profile->min_div_exp)
        TXQ_SCHED_REGISTER_SET(TM_Sched_BlvlTokenBucketTokenEnDiv , MaxDivExp ,    profile->max_div_exp)             
        
        TXQ_SCHED_REGISTER_SET(TM_Sched_BlvlTokenBucketTokenEnDiv , MinToken ,     profile->min_token)
        TXQ_SCHED_REGISTER_SET(TM_Sched_BlvlTokenBucketTokenEnDiv , MaxToken ,     profile->max_token )
        TXQ_SCHED_REGISTER_SET(TM_Sched_BlvlTokenBucketTokenEnDiv , MinTokenRes ,profile->min_token_res*ctl->level_data[B_LEVEL].token_res_exp)
        TXQ_SCHED_REGISTER_SET(TM_Sched_BlvlTokenBucketTokenEnDiv , MaxTokenRes ,profile->max_token_res*ctl->level_data[B_LEVEL].token_res_exp) 
         
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.BlvlTokenBucketTokenEnDiv, node_ind, TM_Sched_BlvlTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.BlvlTokenBucketBurstSize, node_ind,  TM_Sched_BlvlTokenBucketBurstSize)
        if (rc) goto out;
        /* assign register fields */        
        TXQ_SCHED_REGISTER_SET(TM_Sched_BlvlTokenBucketBurstSize , MaxBurstSz , profile->max_burst_size)
        TXQ_SCHED_REGISTER_SET(TM_Sched_BlvlTokenBucketBurstSize , MinBurstSz , profile->min_burst_size)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.BlvlTokenBucketBurstSize, node_ind,  TM_Sched_BlvlTokenBucketBurstSize)
        if (rc) goto out;
        break;
    case C_LEVEL:
        NODE_VALIDATION(ctl->tm_total_c_nodes)
        if (rc) goto out;
        /* profile assignment */
        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.ClvlTokenBucketTokenEnDiv, node_ind, TM_Sched_ClvlTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* assign register fields */        
        TXQ_SCHED_REGISTER_SET(TM_Sched_ClvlTokenBucketTokenEnDiv , MinDivExp ,    profile->min_div_exp)
        TXQ_SCHED_REGISTER_SET(TM_Sched_ClvlTokenBucketTokenEnDiv , MaxDivExp ,    profile->max_div_exp)
        
        TXQ_SCHED_REGISTER_SET(TM_Sched_ClvlTokenBucketTokenEnDiv , MinToken ,   profile->min_token )
        TXQ_SCHED_REGISTER_SET(TM_Sched_ClvlTokenBucketTokenEnDiv , MaxToken ,   profile->max_token )
        TXQ_SCHED_REGISTER_SET(TM_Sched_ClvlTokenBucketTokenEnDiv , MinTokenRes ,profile->min_token_res*ctl->level_data[C_LEVEL].token_res_exp)
        TXQ_SCHED_REGISTER_SET(TM_Sched_ClvlTokenBucketTokenEnDiv , MaxTokenRes ,profile->max_token_res*ctl->level_data[C_LEVEL].token_res_exp) 
        
/* 
        let prevent case that  eligible function core is changed (in elig_function table)   ,but clients will not be updated
        set PerEn  always enabled for nodes
        TM_REGISTER_SET(TM_Sched_ClvlTokenBucketTokenEnDiv , PerEn ,        1)
*/
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.ClvlTokenBucketTokenEnDiv, node_ind, TM_Sched_ClvlTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.ClvlTokenBucketBurstSize, node_ind, TM_Sched_ClvlTokenBucketBurstSize)
        if (rc) goto out;
        /* assign register fields */        
        TXQ_SCHED_REGISTER_SET(TM_Sched_ClvlTokenBucketBurstSize , MaxBurstSz , profile->max_burst_size)
        TXQ_SCHED_REGISTER_SET(TM_Sched_ClvlTokenBucketBurstSize , MinBurstSz , profile->min_burst_size)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.ClvlTokenBucketBurstSize, node_ind, TM_Sched_ClvlTokenBucketBurstSize)
        if (rc) goto out;
        break;
#endif        
    default:
        rc = -EFAULT;
        break;
    }
out:
    return rc;
}



int prvSchedLowLevelSetPortShapingParametersToHw(PRV_CPSS_SCHED_HANDLE hndl, uint8_t port_ind)
{
    #if 0
    int rc =  -EFAULT;
    struct tm_port *port = NULL;
    #endif

    /* avoid compilation errors/warnings */
    hndl = hndl;
    port_ind = port_ind;

    /*no shaping on ports*/                    
      return 0;        

#if 0     

    TM_REGISTER_VAR(TM_Sched_PortTokenBucketTokenEnDiv)
    TM_REGISTER_VAR(TM_Sched_PortTokenBucketBurstSize)

    TM_CTL(ctl, hndl)

                 
     
                     


    if (port_ind < ctl->tm_total_ports)
    {
        port = &(ctl->tm_port_array[port_ind]);

        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.PortTokenBucketTokenEnDiv , port_ind , TM_Sched_PortTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* assign register fields */        
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortTokenBucketTokenEnDiv , Periods ,      port->periods)
        

        TXQ_SCHED_REGISTER_SET(TM_Sched_PortTokenBucketTokenEnDiv , MinToken ,      port->cir_token)
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortTokenBucketTokenEnDiv , MaxToken ,      port->eir_token )
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortTokenBucketTokenEnDiv , MinTokenRes ,   ctl->level_data[P_LEVEL].token_res_exp)
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortTokenBucketTokenEnDiv , MaxTokenRes ,   ctl->level_data[P_LEVEL].token_res_exp) 

        
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.PortTokenBucketTokenEnDiv , port_ind , TM_Sched_PortTokenBucketTokenEnDiv)
        if (rc) goto out;

        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.PortTokenBucketBurstSize , port_ind , TM_Sched_PortTokenBucketBurstSize)
        if (rc) goto out;
        /* assign register fields */        
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortTokenBucketBurstSize , MaxBurstSz , port->eir_burst_size)
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortTokenBucketBurstSize , MinBurstSz , port->cir_burst_size)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.PortTokenBucketBurstSize , port_ind , TM_Sched_PortTokenBucketBurstSize)
        if (rc) goto out;
    }
out:
    return rc;
#endif    
}

/* assume here that shaping profile is valid */
int prvSchedLowLevelGetNodeShapingParametersFromHw
(   
    PRV_CPSS_SCHED_HANDLE hndl, 
    enum schedLevel level, 
    uint32_t node_ind, 
    struct prvCpssDxChTxqSchedShapingProfile *profile
 )
{
    int rc = 0;
    
 

    TM_REGISTER_VAR(TM_Sched_AlvlTokenBucketTokenEnDiv)
    TM_REGISTER_VAR(TM_Sched_QueueTokenBucketTokenEnDiv)
    TM_REGISTER_VAR(TM_Sched_AlvlTokenBucketBurstSize)
    TM_REGISTER_VAR(TM_Sched_QueueTokenBucketBurstSize)
 
    TM_CTL(ctl, hndl)
   
    

    switch (level)
    {
    case Q_LEVEL:
        NODE_VALIDATION(ctl->tm_total_queues)
        if (rc) goto out;

        TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.QueueTokenBucketTokenEnDiv, node_ind,TM_Sched_QueueTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_GET(TM_Sched_QueueTokenBucketTokenEnDiv, MinDivExp,   profile->min_div_exp , (uint8_t))
        TXQ_SCHED_REGISTER_GET(TM_Sched_QueueTokenBucketTokenEnDiv, MinToken , profile->min_token , (uint16_t))
         TXQ_SCHED_REGISTER_GET(TM_Sched_QueueTokenBucketTokenEnDiv, MinTokenRes , profile->min_token_res , (uint8_t))


        TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.QueueTokenBucketBurstSize, node_ind,TM_Sched_QueueTokenBucketBurstSize)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_GET(TM_Sched_QueueTokenBucketBurstSize, MinBurstSz,   profile->min_burst_size , (uint16_t))
        
        break;

      case A_LEVEL:
        NODE_VALIDATION(ctl->tm_total_a_nodes)
        if (rc) goto out;

        TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.AlvlTokenBucketTokenEnDiv, node_ind,TM_Sched_AlvlTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_GET(TM_Sched_AlvlTokenBucketTokenEnDiv, MinDivExp,   profile->min_div_exp , (uint8_t))
        TXQ_SCHED_REGISTER_GET(TM_Sched_AlvlTokenBucketTokenEnDiv, MinToken , profile->min_token , (uint16_t))
         TXQ_SCHED_REGISTER_GET(TM_Sched_AlvlTokenBucketTokenEnDiv, MinTokenRes , profile->min_token_res , (uint8_t))


        TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.AlvlTokenBucketBurstSize, node_ind,TM_Sched_AlvlTokenBucketBurstSize)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_GET(TM_Sched_AlvlTokenBucketBurstSize, MinBurstSz,   profile->min_burst_size , (uint16_t))
        
        break;
     
    
    default:
        rc = -EFAULT;
        break;
    }
out:
    return rc;
}

