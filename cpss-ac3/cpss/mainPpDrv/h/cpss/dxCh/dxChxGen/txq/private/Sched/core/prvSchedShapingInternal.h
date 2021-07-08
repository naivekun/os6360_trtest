/*
 * (c), Copyright 2009-2013, Marvell International Ltd.  (Marvell)
 *
 * This code contains confidential information of Marvell.
 * No rights are granted herein under any patent, mask work right or copyright
 * of Marvell or any third party. Marvell reserves the right at its sole
 * discretion to request that this code be immediately returned to Marvell.
 * This code is provided "as is". Marvell makes no warranties, expressed,
 * implied or otherwise, regarding its accuracy, completeness or performance.
 */
/**
 * @brief internal API for TM TM Shaping configuration.
 *
* @file tm_shaping_internal.h
*
* $Revision: 2.0 $
 */

#ifndef     SCHED_SHAPING_INTERNAL_H
#define     SCHED_SHAPING_INTERNAL_H

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedCoreTypes.h>


/* reserves and initializes default shaping profile - SCHED_INF_SHP_PROFILE */
int prvCpssSchedShapingInitInfiniteShapingProfile(PRV_CPSS_SCHED_HANDLE hndl);

/* internal utilities for managing clients of shaping profiles */
							  
int prvCpssSchedShapingCheckShapingProfileValidity(	PRV_CPSS_SCHED_HANDLE hndl,uint32_t prof_index, enum schedLevel level);
							  
int prvCpssSchedShapingAddNodeToShapingProfile(PRV_CPSS_SCHED_HANDLE hndl, uint32_t prof_index,enum schedLevel level,uint32_t node_index);

int prvCpssSchedShapingRemoveNodeFromShapingProfile(PRV_CPSS_SCHED_HANDLE hndl, uint32_t prof_index,enum schedLevel level,uint32_t node_index);
							  
#ifdef PER_NODE_SHAPING

	int prvCpssSchedShapingUpdateNodeShapingProc(PRV_CPSS_SCHED_HANDLE hndl,
									  enum schedLevel level,
									  uint32_t node_index,
									  struct tm_shaping_profile_params * node_shaping_params);

	int prvCpssSchedShapingUpdateNodeShapingProRreadNodeShapingProc(PRV_CPSS_SCHED_HANDLE hndl,
								  enum schedLevel level,
								  uint32_t node_index,
								  uint16_t * shaping_profile,
								  struct tm_shaping_profile_params * node_shaping_params);
#endif							  
#endif   /* PRV_TM_SHAPING_H */
