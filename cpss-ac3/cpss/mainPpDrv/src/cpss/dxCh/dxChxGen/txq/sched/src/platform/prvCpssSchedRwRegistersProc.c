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
 * @brief basic H/W read wtite utilities for read/write registers implementation for cpss
 *
* @file cpss_hw_registers_proc.c
*
* $Revision: 2.0 $
 */
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedOsInterface.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedCpssRwRegistersProc.h>

#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpss/generic/cpssHwInit/private/prvCpssHwRegisters.h>

#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedOsInterface.h>



#include <stdio.h>


#define TM_GLOBAL_ADDR       0xC000000
#define TM_GLOBAL_REG        0xC0FFFF8
#define TM_GLOBAL_READ_REG   0xC0FFFF0



#define FALCON_TM_GLOBAL_REG        0x0FFFFF8



/* #define WRITE_REGISTERS_DUMP  */

#ifdef WRITE_REGISTERS_DUMP
    CPSS_OS_FILE    fregistersDump=CPSS_OS_FILE_INVALID;
    int     stringIndex=0;
#endif

/* #define RW_TEST  */
/* #define RW_TEST_LOG  */

#ifdef RW_TEST_LOG
    #define RW_TEST
    FILE *  fRWerrorLog=NULL;
#endif



int schedLogWriteRequests = 0;
int schedLogReadRequests = 0;
void *  sched_pAliasingDummy=NULL;

/**
 */
int prvSchedReadRegisterProc
(
    GT_U8 devNumber,
    GT_U64 reg_addr,  
    void *data,
    GT_U32 unitBaseAdress
)
{
    GT_STATUS   ret;
    GT_U32      regAddr[2];
    GT_U32      regValue[2];
 

    regAddr[0] = reg_addr.l[0] + unitBaseAdress;
    regAddr[1] = regAddr[0]+4;


    ret = prvCpssHwPpReadRegister(devNumber, regAddr[0], &(regValue[0])); /* LSB */
    if (GT_OK != ret) return ret;

    ret = prvCpssHwPpReadRegister(devNumber, regAddr[1], &(regValue[1])); /* MSB */
    
    if (GT_OK != ret) return ret;

    ((GT_U64 *)data)->l[0] = regValue[0];
    ((GT_U64 *)data)->l[1] = regValue[1];

    return ret;
}



int prvSchedWriteRegisterProc
(
    GT_U8 devNumber,
    GT_U64 reg_addr,  
    void *data,
    GT_U32 unitBaseAdress
)
{
    GT_STATUS   ret;
    GT_U32      regAddr[2];
    GT_U32      regValue[2];
    
    regAddr[0] = reg_addr.l[0] + unitBaseAdress;
    regAddr[1] = regAddr[0]+4;
   

    regValue[0] = ((GT_U64 *)data)->l[0];
    regValue[1] = ((GT_U64 *)data)->l[1];
       
         
    ret = prvCpssHwPpWriteRam(devNumber, regAddr[0], 1, &(regValue[0]));
    if (GT_OK != ret) return ret;

    ret = prvCpssHwPpWriteRam(devNumber, regAddr[1], 1, &(regValue[1]));
        
    return ret;
}




/*
*/


int prvSchedPrintRxRegistersLogStatus(void)
{
    schedPrintf(" --------------------\n");
    schedPrintf("\n  print write calls : ");
    schedPrintf(schedLogWriteRequests ? "yes" : "no");
    schedPrintf("\n  print read  calls : ");
    schedPrintf(schedLogReadRequests ? "yes" : "no");
    schedPrintf("\n");
    return 0;
}


int prvSchedSetRegistersLogStatus(int write_status, int read_status)
{
    switch (write_status)
    { 
        case 0:
        case 1:break;
        default : schedPrintf("wrong write flag value , abort ...\n"); return 1;
    }
    switch (read_status)
    { 
        case 0:
        case 1:break;
        default : schedPrintf("wrong read flag value , abort ...\n"); return 1;
    }
    schedLogWriteRequests = write_status;
    schedLogReadRequests = read_status;
    return prvSchedPrintRxRegistersLogStatus();
}
