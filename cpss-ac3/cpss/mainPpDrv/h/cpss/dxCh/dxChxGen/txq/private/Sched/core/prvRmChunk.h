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
 * @brief rm_chunk interface.
 *
* @file rm_chunk.h
*
* $Revision: 2.0 $
 */

#ifndef SCHED_RM_CHUNK_H
#define SCHED_RM_CHUNK_H

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvRmInterface.h>
/* for definition of schedLevel */
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedDefs.h>




typedef struct rm_chunk *chunk_ptr;


/***************************************************************
  new chunk storage
***************************************************************/




/** Find index of matching chunk and update internal DB.
 * 
 *   @param[in]		hndl		      Resource Manager handle.	
 *   @param[in]		lvl		      Hierarchy level. 
 *   @param[in]		size		      Length of needed range.
 *   @param[out]	index		      Index of node.	
 *
 *   @return an integer return code.
 *   @retval zero on success. 
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF if hndl is an invalid handle.
 *   @retval -ENOMEM if no free space in needed size.
 */
int			prvSchedRmAllocateChunk(rmctl_t hndl, enum schedLevel lvl, uint32_t size, uint32_t *index);


/** if range first_index::first_index+size is free , allocates it  and update internal DB.
 * 
 *   @param[in]		hndl		      Resource Manager handle.	
 *   @param[in]		lvl		          Hierarchy level. 
 *   @param[out]	index		      first index of range.	
 *   @param[in]		size		      Length of needed range.
 *
 *   @return an integer return code.
 *   @retval zero on success. 
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF if hndl is an invalid handle.
 *   @retval -ENOMEM if required range is occupied.
 */
int			prvSchedRmGetChunk(rmctl_t hndl, enum schedLevel lvl, uint32_t first_index, uint32_t size);



/** Add chunk of the nodes that became free and update internal DB.
 * 
 *   @param[in]		hndl			  Resource Manager handle.	
 *   @param[in]		lvl		          Hierarchy level. 
 *   @param[in]		size		      Length of needed range.
 *   @param[in]		index		      Index of node.	
 *
 *   @return an integer return code.
 *   @retval zero on success. 
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF if hndl is an invalid handle.
 *   @retval -ENOMEM when out of memory space.
 */
int			prvSchedRmReleaseChunk(rmctl_t hndl, enum schedLevel lvl, uint32_t size, uint32_t index);


/** Find free chunk that includes index and get from it node to expand used nodes range.
 * 
 *   @param[in]		hndl          Resource Manager handle.	
 *   @param[in]		lvl		      Hierarchy level. 
 *   @param[in]	    index		  Index of node.	
 *
 *   @return an integer return code.
 *   @retval zero on success. 
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF if hndl is an invalid handle.
 *   @retval -ENOMEM if index not found.
 */
int			prvSchedRmExpandChunkRight(rmctl_t hndl, enum schedLevel lvl, uint32_t index);

int			prvSchedRmExpandChunkLeft(rmctl_t hndl, enum schedLevel lvl, uint32_t index);


/** create new chunk and add it to free chunk list ( list elements are stay in decending order of chunk size)
 * 
 *   @param[in]	    index		  Index of starting node.	
 *   @param[in]	    length		  length of chunk
 *   @param[in]	    chunk_list	  Pointer to chunk list to add to
 *
 *   @return  new pointer to updated chunk list
 */
chunk_ptr	prvSchedRmNewChunk(uint32_t start_index, uint32_t length, struct rm_chunk * chunk_list);


/** deallocate chunk list and releases allocated memory
 * 
 *   @param[in]	    chunk_list	  Pointer to chunk list to free *
 */
void		prvSchedRmClearChunkList(chunk_ptr list);

int32_t	prvSchedRmGetMaxChunkSize(rmctl_t hndl, enum schedLevel lvl);


/* debugging utilities */

/* prints dump of chunk list for debugging proposes
*/
int prvSchedRmDumpChunk(rmctl_t hndl, enum schedLevel lvl);

/* following function return content of N-th chunk in the list - index & size
*  for ut creation 
*  if chunk exists - returned value = 0 & index & size contains data from chunk,
*  otherwise  1 is returned
*/
int prvSchedRmGetChunkData(rmctl_t hndl, enum schedLevel lvl, int chunk_number_in_list, uint32_t *index , uint32_t * size);




#endif                          /* RM_CHUNK_H */


