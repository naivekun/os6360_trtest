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
 * @brief Resource Manager control functions implementation.
 *
* @file rm_ctl.c
*
* $Revision: 2.0 $
 */

#include <stdlib.h>
#include <errno.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvRmInternalTypes.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvRmCtl.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvRmChunk.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedOsInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedCoreTypes.h>


static int  init_resource_manager(struct rm_resource_manager * mgr, uint32_t size);
static void free_resource_manager(struct rm_resource_manager * mgr);







/* for TM_NUM_PORT_DROP_PROF definition */
/*#include "tm_hw_configuration_interface.h" */


/**
 */
 
#define CHECK_RM_CREATION(fun)   if (!fun) { prvSchedRmClose((rmctl_t)ctl); return NULL;}
 
 
rmctl_t * prvSchedRmOpen
(
    uint32_t total_c_nodes,
    uint32_t total_b_nodes,
    uint32_t total_a_nodes,
    uint32_t total_queues
)
{

    struct rmctl *   ctl = NULL;

    /* Create rmctl instance */
    ctl = schedMalloc(sizeof(struct rmctl));
    if (ctl) 
    {
        schedMemSet(ctl, 0, sizeof(*ctl));

        /* Fill in ctl structure */
        ctl->magic = RM_MAGIC;
        
        CHECK_RM_CREATION(init_resource_manager(&(ctl->rm_profiles[RM_SHAPING_PRF]), total_queues + total_a_nodes + total_b_nodes + total_c_nodes))

        


        ctl->rm_free_nodes[A_LEVEL]=prvSchedRmNewChunk(0,total_a_nodes,NULL);
        ctl->rm_free_nodes[B_LEVEL]=prvSchedRmNewChunk(0,total_b_nodes,NULL);
        ctl->rm_free_nodes[C_LEVEL]=prvSchedRmNewChunk(0,total_c_nodes,NULL);

#if 0       
        if(is_sip_6 == 0)
        {
            /* special case - patching H/W bug  - exclude queue 0  from configuration*/
            ctl->rm_free_nodes[Q_LEVEL]=prvSchedRmNewChunk(1,total_queues-1,NULL);
        }
#endif        

         /* Looks like this H/W bug is not releavnt to Falcon*/
        ctl->rm_free_nodes[Q_LEVEL]=prvSchedRmNewChunk(0,total_queues-1,NULL);
        
    
    }
    return (rmctl_t *)ctl;
}


/**
 */
int prvSchedRmClose(rmctl_t hndl)
{
    int i;

    RM_HANDLE(ctl, hndl);

    ctl->magic = 0;

    prvSchedRmClearChunkList(ctl->rm_free_nodes[Q_LEVEL]);
    prvSchedRmClearChunkList(ctl->rm_free_nodes[A_LEVEL]);
    prvSchedRmClearChunkList(ctl->rm_free_nodes[B_LEVEL]);
    prvSchedRmClearChunkList(ctl->rm_free_nodes[C_LEVEL]);


    for (i=0 ; i < RM_MAX_RESOURCE_TYPES ; i++)
    {
        free_resource_manager(&(ctl->rm_profiles[i]));
    }

    /* free rm handle */
    schedFree(ctl);
    return 0;
}


/****************************************************************************
 * resource manager implementation
 ****************************************************************************/ 




int init_resource_manager(struct rm_resource_manager * mgr, uint32_t size)
{
    mgr->used = schedMalloc(size*sizeof(uint8_t));
    /* set all resources free */
    if (mgr->used)
    {
        mgr->size = size;
        schedMemSet(mgr->used, 0, size);
        mgr->counter = size;    
        mgr->index= 0;
    }
    return (mgr->used !=NULL);
}

void free_resource_manager(struct rm_resource_manager * mgr)
{
    if (mgr->used)  schedFree(mgr->used);
}






int  prvCpssRmGetFreeResource(struct rm_resource_manager * mgr)
{
    if (mgr->counter == 0)  return -1;
    while(mgr->used[mgr->index] != RESOURCE_FREE)
    {
        mgr->index++;
        if (mgr->index == mgr->size) mgr->index=0;
    }
    
    mgr->used[mgr->index] = RESOURCE_ALLOCATED; 
    mgr->counter--;
    return mgr->index;
}




int  prvCpssRmReleaseResource(struct rm_resource_manager * mgr, uint32_t resource_id)
{
    if (resource_id >= mgr->size) return -1; /* out of range */
    if (mgr->used[resource_id] != RESOURCE_ALLOCATED)  return -1; /* not allocated  or range allocated*/
    mgr->used[resource_id] = RESOURCE_FREE;
    mgr->counter++;
    /* hole processing */
    if (mgr->index > resource_id) mgr->index = resource_id;
    return 0;
}



int  prvCpssRmResourceStatus(struct rm_resource_manager * mgr, uint32_t resource_id)
{
    if (resource_id >= mgr->size)  return  -1;
    return mgr->used[resource_id] ;
}







int  prvCpssRmValidateResourceRange(struct rm_resource_manager * mgr, uint32_t resource_id, uint32_t range)
{
    uint32_t i;
    if (resource_id >= mgr->size) return 1; /* invalid */
    for (i =0 ; i < range ; i++) 
    {
        if (mgr->used[i+resource_id] != RESOURCE_RANGE_ALLOCATED+i ) return 1;
    }
    return 0;
}



