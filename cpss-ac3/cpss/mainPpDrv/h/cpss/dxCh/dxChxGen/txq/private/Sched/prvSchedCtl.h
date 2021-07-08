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
 * @brief TM Configuration Library Control Interface
 *
* @file prvCpssDxChTxqSchedShadow.h
*
* $Revision: 2.0 $
 */

#ifndef   	SCHED_CTL_H
#define   	SCHED_CTL_H

#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedDefs.h>
#include <cpss/common/cpssTypes.h>




/** Customize Initialize TM configuration library.
 * 
 *   @param[in]     devNum          HW Device Number
 *   @param[in]     init_params_ptr struct of PRV_CPSS_SCHED_LIB_INIT_PARAMS_STC
 *   @param[out]	hndl		    TM lib handle
 *
 *   @return an integer return code.
 *   @retval zero on success. 
 *   @retval -EINVAL if any of handles is NULL.
 *   @retval -EBADF if any of handles is invalid.
 *   @retval -ENOMEM when out of memory space.
 *
 *   @retval TM_CONF_INVALID_PROD_NAME.
 *   @retval TM_CONF_CANNT_GET_LAD_FREQUENCY.
 *   @retval TM_HW_GEN_CONFIG_FAILED.
 */
int prvSchedLibOpenExt(
	GT_U8 devNum,
	struct PRV_CPSS_SCHED_TREE_MAX_SIZES_STC *tree_struct,
	struct PRV_CPSS_SCHED_LIB_INIT_PARAMS_STC *init_params_ptr, 
	PRV_CPSS_SCHED_HANDLE * pHndl);


#endif   /* TM_CTL_H */

