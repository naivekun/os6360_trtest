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
 * @brief interface to functions for reset TM S/W DB values
 *
* @file set_local_db_defaults.h
*
* $Revision: 2.0 $
 */

#ifndef SCHED_SET_LOCAL_DB_DEFAULTS_H_
#define SCHED_SET_LOCAL_DB_DEFAULTS_H_


#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedCoreTypes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvRmInternalTypes.h>





int prvSchedSetSwConfDefault(void * hndl);
int prvSchedSetSwGenConfDefault(void * hndl);

int	prvSchedSetSwShapingProfileDefault(struct prvCpssDxChTxqSchedShapingProfile *profiles,
                                    uint32_t prof_index);


                                            
void prvSchedSetSw_queue_Default(struct tm_queue * queue_node, int bIncludeMapping);
void prvSchedSetSw_a_node_Default(struct tm_a_node * node, int bIncludeMapping);
void prvSchedSetSw_b_node_Default(struct tm_b_node * node, int bIncludeMapping);
void prvSchedSetSw_c_node_Default(struct tm_c_node * node, int bIncludeMapping);
void prvSchedSetSwPortDefault(struct tm_port * port_node, int bIncludeMapping);

/* initializing default values for eligible functions tables*/
void prvSchedInitDefaultNodeEligPrioFuncTable(struct tm_elig_prio_func_node *func_table);
void prvSchedInitDefaultQueueEligPrioFuncTable(struct tm_elig_prio_func_queue  *func_table);
                                                  
int prvSchedIsQueueEligFuncUseShaper(struct tm_elig_prio_func_queue * queue_func_table,uint8_t func_index);

int prvSchedIsNodeEligFuncUseShaper(struct tm_elig_prio_func_node * node_func_table, uint8_t func_index);
                                                  
/* Auxiliary function */

void prvSchedConvertValueToEligFunc(uint16_t elig_val, struct tm_elig_prio_func_out *elig_func);

void prvSchedSetSwQEligPrioFunc(struct tm_elig_prio_func_queue *func_table, 
                                   uint16_t func_offset,
                                   union tm_elig_prio_func *queue_func_value_arr);

void prvSchedSetSwNodeEligPrioFunc(struct tm_elig_prio_func_node *func_table, 
                                   uint16_t func_offset,
                                   union tm_elig_prio_func *node_func_value_arr);

#endif /* _SET_LOCAL_DB_DEFAULTS_H_ */
