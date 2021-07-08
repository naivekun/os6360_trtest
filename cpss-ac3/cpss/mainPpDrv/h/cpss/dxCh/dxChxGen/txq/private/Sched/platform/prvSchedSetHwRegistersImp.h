#ifndef _SET_HW_REGISTERS_IMP_H_
#define _SET_HW_REGISTERS_IMP_H_


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
 * @brief useful macro wrappers for read/write/reset register functions.
 *
* @file set_hw_registers_imp.h
*
* $Revision: 2.0 $
 */



#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedRegistersInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedOsInterface.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>



#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedRegs.h>

#define SHIFT_TABLE
/*#define STRUCTS */


#if defined(SHIFT_TABLE)



#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvShedRegsDescription.h>

    


    /*#include "falcon/tm_regs_description.h"*/


#else

#endif
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedRegistersProcessing.h>





#if __STDC_VERSION__ < 199901L
# if __GNUC__ >= 2
#  define __func__ __FUNCTION__
# else
#  define __func__ "<unknown>"
# endif
#endif






#if defined(STRUCTS)
	#define	TXQ_SCHED_RESET_REGISTER(address, register_name)	\
	/*	schedPrintf(" function %s -  reset register reserved fields %s(0x%08X)... \n",__FUNCTION__,#address,address);*/\
		/*rc = tm_register_reset(TM_ENV(ctl), REGISTER_ADDRESS_ARG(address),  TM_REGISTER_VAR_ADDR(register_name));*/\
		/**/if (rc) schedPrintf(" function %s -  failed to reset reserved fields for register %s(0x%08X) \n",__func__,#address,address);/**/
#elif defined(SHIFT_TABLE)
	#define	TXQ_SCHED_RESET_REGISTER(address, register_name)	\
	TXQ_SCHED_REGISTER_RESET(register_name)\
	rc = 0;
#else
	#define	TXQ_SCHED_RESET_REGISTER(address, register_name)  /* do nothing */
#endif


#if defined(STRUCTS)
	#define	TM_RESET_TABLE_REGISTER(address, index , register_name)	\
	/*	schedPrintf(" function %s -  reset table register reserved fields %s(base address 0x%08X , shift %d)  value 0x%016X\n",__FUNCTION__,#address,address,index, *((uint64_t*)TM_REGISTER_VAR_ADDR(register_name)));*/\
		/*rc = tm_table_entry_reset(TM_ENV(ctl), REGISTER_ADDRESS_ARG(address), index, TM_REGISTER_VAR_ADDR(register_name));*/\
		/**/if (rc) schedPrintf(" function %s -  failed to reset reserved fields to table  register %s(0x%08X) , index %ld \n",__func__,#address,address,(long int)index);/**/
#elif defined(SHIFT_TABLE)
	#define	TM_RESET_TABLE_REGISTER(address, index , register_name)	\
	TXQ_SCHED_REGISTER_RESET(register_name)\
	rc = 0;
#else
	#define	TM_RESET_TABLE_REGISTER(address,index , register_name)  /* do nothing */
#endif

extern int schedLogWriteRequests;
extern int schedLogReadRequests;
extern void *  sched_pAliasingDummy;

#define	TXQ_SCHED_WRITE_REGISTER(address, register_name)	\
	if(schedLogWriteRequests)\
	{\
		sched_pAliasingDummy = (void*)TM_REGISTER_VAR_ADDR(register_name);\
		schedPrintf(" function %s -  writing to register %s(0x%08X %08X) value 0x%016llX\n",__FUNCTION__,#address,address.l[1],address.l[0],*((uint64_t*)sched_pAliasingDummy));\
	}\
	/*	schedPrintf(" function %s -  writing to register %s value 0x%016llX\n",__FUNCTION__,#address,*((uint64_t*)TM_REGISTER_VAR_ADDR(register_name)));*/\
	rc = prvSchedRegisterWrite(TM_ENV(ctl), REGISTER_ADDRESS_ARG(address),  TM_REGISTER_VAR_ADDR(register_name));\
    /**/if (rc) schedPrintf(" function %s -  failed writing to register %s(0x%08X %08X) \n",__func__,#address,address.l[1],address.l[0]);/**/

#define	TXQ_SCHED_READ_REGISTER(address, register_name)	\
	if(schedLogReadRequests) 	schedPrintf(" function %s -  reading from register %s(0x%08X %08X) ... ",__FUNCTION__, #address, address.l[1], address.l[0]);\
	rc = prvSchedRegisterRead(TM_ENV(ctl), REGISTER_ADDRESS_ARG(address),  TM_REGISTER_VAR_ADDR(register_name));\
	if(schedLogReadRequests)\
	{\
		if (rc==0)\
		{\
			sched_pAliasingDummy = (void*)TM_REGISTER_VAR_ADDR(register_name);\
			schedPrintf(" result 0x%016llX\n",*((uint64_t*)sched_pAliasingDummy));\
		}\
		else schedPrintf(" failed !\n");\
	} \
	/**/if (rc) schedPrintf(" function %s -  failed to read from register %s(0x%08X %08X) \n",__func__,#address,address.l[1],address.l[0]);/**/

#define	TXQ_SCHED_WRITE_TABLE_REGISTER(address, index , register_name)	\
	if(schedLogWriteRequests)\
	{\
		sched_pAliasingDummy = (void*)TM_REGISTER_VAR_ADDR(register_name);\
		schedPrintf(" function %s -  writing to table register %s(base address 0x%08X %08X) , shift %ld)  value 0x%016llX\n",__FUNCTION__,#address,address.l[1],address.l[0],index, *((uint64_t*)sched_pAliasingDummy));\
	}\
	/*	schedPrintf(" function %s -  writing to table register %s , shift %ld  value 0x%016llX\n",__FUNCTION__, #address, index, *((uint64_t*)TM_REGISTER_VAR_ADDR(register_name)));*/\
	rc = prvSchedTableEntryWrite(TM_ENV(ctl), REGISTER_ADDRESS_ARG(address), index, TM_REGISTER_VAR_ADDR(register_name));\
    /**/if (rc) schedPrintf(" function %s -  failed writing to table  %s (0x%08X %08X) , index %ld \n",__func__,#address,address.l[1],address.l[0],(long int)index);/**/

#define	TXQ_SCHED_READ_TABLE_REGISTER(address, index , register_name)	\
	if(schedLogReadRequests) 	schedPrintf(" function %s -  reading from  table register %s(base address 0x%08X %08X , shift %d) ...",__FUNCTION__,#address,address.l[1],address.l[0],index);\
	rc = prvSchedTableEntryRead(TM_ENV(ctl), REGISTER_ADDRESS_ARG(address), index, TM_REGISTER_VAR_ADDR(register_name));\
	if(schedLogReadRequests)\
	{\
		if (rc==0)\
		{\
			sched_pAliasingDummy = (void*)TM_REGISTER_VAR_ADDR(register_name);\
			schedPrintf(" result 0x%016llX\n",*((uint64_t*)sched_pAliasingDummy));\
		}\
		else schedPrintf(" failed !\n");\
	} \
    /**/if (rc) schedPrintf(" function %s -  failed to read from table %s (0x%08X %08X) , index %ld \n",__func__,#address,address.l[1],address.l[0],(long int)index);/**/



#endif  /* _SET_HW_REGISTERS_IMP_H_ */
