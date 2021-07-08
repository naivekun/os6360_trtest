#ifndef _SCHED_HARDWARE_CONFIGURATION_INTERFACE_H_
#define _SCHED_HARDWARE_CONFIGURATION_INTERFACE_H_

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
 * @brief declaration of interface to H/W configuration.
 *
* @file tm_hw_configuration_interfase.h
*
* $Revision: 2.0 $
 */

#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedDefs.h>

/*
 interface for access to all hardware resources - interface is platform independent, 
 implementation is platform dependent, different files for different platforms
*/

/*
 initialization of all hardware resources
 returning result - 0 if success
                    >0 if failed
*/



unsigned int	prvCpssSchedInitHardwareConfiguration(void * hEnv);


int prvCpssSchedSetGlobalAddress(void * hEnv,GT_U32 pdqNum,GT_U32 globalAddress);




/*
 following function returns 1 if hardware resources were successfully initialized 
 otherwise 0 (finitialization failed or not performed)
*/

/* following functions return appropriate hardware value  or 0 if hardware initialization missed or failed */
int	prvCpssSchedGetPortCount(void * hEnv);
int	prvCpssSchedGetCnodesCount(void * hEnv);
int	prvCpssSchedGetBnodesCount(void * hEnv);
int	prvCpssSchedGetAnodesCount(void * hEnv);
int	prvCpssSchedGetQNodesCount(void * hEnv);


unsigned int	prvCpssSchedGetNodeQuantumChunkSize(); 
unsigned int	prvCpssSchedGetNodeQuantumRange();
unsigned int	prvCpssSchedGetPortQuantumChunkSize(); 
unsigned int	prvCpssSchedGetNodeMaxQuantumChunks();
unsigned int	prvCpssSchedGetPortMaxQuantumChunks();

#endif /* _SCHED_HARDWARE_CONFIGURATION_INTERFACE_H_ */
