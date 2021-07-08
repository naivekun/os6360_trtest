#ifndef _TM_CORE_RM_INTERNAL_TYPES_H_
#define _TM_CORE_RM_INTERNAL_TYPES_H_

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
 * @brief rm internal DB and definitions.
 *
* @file rm_internal_types.h
*
* $Revision: 2.0 $
 */


#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvShedPlatformImplementationDefinitions.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvRmChunk.h>


/* nesessary fo RM_HANDLE macro */
#include <errno.h>

/** Magic number used to determine struct rmctl_t validity.
 */
#define RM_MAGIC 0x1EDA5D


/* following macro declares and checks validity of rmctl */
#define DECLARE_RM_HANDLE(handle,value)	struct rmctl *handle = (struct rmctl *)value;

#define CHECK_RM_HANDLE(handle)	\
	if (!handle) return -EINVAL;\
    if (handle->magic != RM_MAGIC) return -EBADF;

#define RM_HANDLE(handle,value)	DECLARE_RM_HANDLE(handle,value);CHECK_RM_HANDLE(handle);






/** 
 */
enum rm_prf_level {
    RM_SHAPING_PRF = 0,
    RM_MAX_RESOURCE_TYPES,
};


#define RESOURCE_FREE				0
#define RESOURCE_ALLOCATED			1
#define RESOURCE_RANGE_ALLOCATED	2


#define AGING_RESOURCE_SLOT_SIZE	 4


struct rm_resource_manager
{
	uint32_t size;
	uint32_t counter;
	uint32_t index;
    uint8_t  *  used;
} __ATTRIBUTE_PACKED__;


int  prvCpssRmGetFreeResource(struct rm_resource_manager * mgr);
int  prvCpssRmReleaseResource(struct rm_resource_manager * mgr, uint32_t resource_id);
int  prvCpssRmResourceStatus(struct rm_resource_manager * mgr, uint32_t resource_id);
int  prvCpssRmValidateResourceRange(struct rm_resource_manager * mgr, uint32_t resource_id, uint32_t range);


/** returned values : 
 * RESOURCE_FREE - 			  resource free
 * RESOURCE_ALLOCATED - 	  resource allocated
 * RESOURCE_RANGE_ALLOCATED - resource allocated by range
 * -1 - invalid resource_id 
 */ 
int  prvCpssRmResourceStatus(struct rm_resource_manager * mgr, uint32_t resource_id);




/** Resource Manger handle struct.
 */
struct rmctl
{
	uint32_t magic;
	chunk_ptr  rm_free_nodes[4]; /* from Q_LEVEL  to C_LEVEL,  P_LEVEL  not used inside RM */
	struct rm_resource_manager		rm_profiles[RM_MAX_RESOURCE_TYPES];    
};


#endif   /* RM_TYPES_H */
