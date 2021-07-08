/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file osVxIntr.c
*
* @brief VxWorks Operating System wrapper. Interrupt facility.
*
* @version   4
********************************************************************************
*/

#include <intLib.h>
#include <logLib.h>
#include <taskLib.h>
#include <vxWorks.h>

#include <gtOs/gtOsIntr.h>
#include <gtOs/gtOsGen.h>

#ifdef RTOS_ON_SIM
    #include <asicSimulation/SCIB/scib.h>
    #include <common/SHOST/GEN/INTR/EXP/INTR.H>

    #define PROTECT_TASK_DOING_READ_WRITE_START_MAC         SCIB_SEM_TAKE

    #define PROTECT_TASK_DOING_READ_WRITE_END_MAC           SCIB_SEM_SIGNAL
#endif /*RTOS_ON_SIM*/
/************ Extern functions ************************************************/
extern GT_STATUS bspIntEnable(IN GT_U32 intMask);
extern GT_STATUS bspIntDisable(IN GT_U32 intMask);
extern GT_STATUS bspIntConnect(IN GT_U32 irq,IN GT_VOIDFUNCPTR rtn, IN GT_U32 param);
GT_VOID prvDebugCpuUtilizationFromInterrupt(GT_BOOL endOfInt);

static GT_VOIDFUNCPTR intRoutine = NULL;

/**
* @internal intRoutineWrapper function
* @endinternal
*
* @brief   ISR wrapper
*
* @param[in] parameter
*/
GT_VOID intRoutineWrapper
(
    IN GT_U32 parameter
)
{
    prvDebugCpuUtilizationFromInterrupt(GT_FALSE);
    (*intRoutine)(parameter);
    prvDebugCpuUtilizationFromInterrupt(GT_TRUE);
}

/************ Public Functions ************************************************/
/**
* @internal osIntEnable function
* @endinternal
*
* @brief   Enable corresponding interrupt bits
*
* @param[in] intMask                  - new interrupt bits
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osIntEnable
(
    IN GT_U32   intMask
)
{
    IS_WRAPPER_OPEN_STATUS;

#ifndef RTOS_ON_SIM
    return bspIntEnable(intMask);
#else /*RTOS_ON_SIM*/
  SHOSTG_interrupt_enable_one(intMask + 1);
  return GT_OK ;
#endif /*RTOS_ON_SIM*/
}

/**
* @internal osIntDisable function
* @endinternal
*
* @brief   Disable corresponding interrupt bits.
*
* @param[in] intMask                  - new interrupt bits
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osIntDisable
(
    IN GT_U32   intMask
)
{
    IS_WRAPPER_OPEN_STATUS;

#ifndef RTOS_ON_SIM
    return bspIntDisable(intMask);
#else /*RTOS_ON_SIM*/
    SHOSTG_interrupt_disable_one(intMask + 1);
    return GT_OK ;
#endif /*RTOS_ON_SIM*/
}

/**
* @internal osSetIntLockUnlock function
* @endinternal
*
* @brief   Lock/unlock interrupts
*
* @param[in] mode                     - interrupt state lock/unlock
* @param[in,out] key                      - if mode is INTR_MODE_UNLOCK, lock  returned by
*                                      preceding interrupt disable call
* @param[in,out] key                      - if mode is INTR_MODE_LOCK lock  for the interrupt
*                                      level
*                                       Lock key for the interrupt level
*/
GT_32 osSetIntLockUnlock
(
    IN      INTERRUPT_MODE mode,
    INOUT   GT_32          *key
)
{
    static int current_mask;

    IS_WRAPPER_OPEN_PTR;

    if (key == NULL)
        return 0;

    switch (mode)
    {
        case INTR_MODE_UNLOCK :
#ifndef RTOS_ON_SIM
            intUnlock(*key);
#else /*RTOS_ON_SIM*/
            SHOSTG_interrupt_enable(*key);
        PROTECT_TASK_DOING_READ_WRITE_END_MAC;
#endif /*RTOS_ON_SIM*/

        break;

        case INTR_MODE_LOCK :
#ifndef RTOS_ON_SIM
            current_mask = intLock();
#else /*RTOS_ON_SIM*/
        PROTECT_TASK_DOING_READ_WRITE_START_MAC;
            current_mask = SHOSTG_interrupt_disable();
        /* the unlock of the 'protection' can be done only during 'tasks unlock'
          see INTR_MODE_UNLOCK */
        /*PROTECT_TASK_DOING_READ_WRITE_END_MAC;*/
#endif /*RTOS_ON_SIM*/

            *key = current_mask;

        break;

        default:
        break;
    }

    return (current_mask);
}

/**
* @internal osInterruptConnect function
* @endinternal
*
* @brief   Connect a specified C routine to a specified interrupt vector.
*
* @param[in] vector                   - interrupt  number to attach to
* @param[in] routine                  -  to be called
* @param[in] parameter                -  to be passed to routine
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osInterruptConnect
(
    IN  GT_U32           vector,
    IN  GT_VOIDFUNCPTR   routine,
    IN  GT_U32           parameter
)
{
#ifndef RTOS_ON_SIM
    IS_WRAPPER_OPEN_STATUS;

    if (intRoutine == NULL)
    {
        intRoutine = routine;
        return bspIntConnect(vector, (GT_VOIDFUNCPTR)intRoutineWrapper, parameter);
    }
    else if(intRoutine == routine)
    {
        return bspIntConnect(vector, (GT_VOIDFUNCPTR)intRoutineWrapper, parameter);
    }
    else
    {
        return bspIntConnect(vector, routine, parameter);
    }

#else /*RTOS_ON_SIM*/
    GT_U32  shostIntVector = vector + 1;
    IS_WRAPPER_OPEN_STATUS;
    /* call SHOST to bind ISR with simulated interrupts
       engine */
    SHOSTG_bind_interrupt_with_param ( 15, shostIntVector,
                                       routine, NULL, parameter);

     return GT_OK;
#endif /*RTOS_ON_SIM*/
}

/**
* @internal osInterruptDisconnect function
* @endinternal
*
* @brief   DisConnect a specified interrupt vector.
*
* @param[in] vector                - interrupt number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osInterruptDisconnect
(
    IN  GT_U32      vector
)
{
    return GT_OK;
}
