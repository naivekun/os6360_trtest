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
* @file osFreeBsdIntr.c
*
* @brief FreeBsd User Mode Operating System wrapper. Interrupt facility.
*
* @version   5
********************************************************************************
*/


#include <gtOs/gtOsIntr.h>
#include <gtOs/gtOsSem.h>

#include <asicSimulation/SCIB/scib.h>
#include <common/SHOST/GEN/INTR/EXP/INTR.H>

/***** Public Functions ************************************************/

/******Local Variables ************************/
static GT_U32 osInterGo = 0;

GT_MUTEX IntLockmutex;

/**
* @internal osIntEnable function
* @endinternal
*
* @brief   Enable corresponding interrupt bits
*
* @param[in] intVecNum                - new interrupt bits
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osIntEnable
(
    IN GT_U32   intVecNum
)
{
  SHOSTG_interrupt_enable_one(intVecNum);

  return GT_OK ;
}

/**
* @internal osIntDisable function
* @endinternal
*
* @brief   Disable corresponding interrupt bits.
*
* @param[in] intVecNum                - new interrupt bits
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osIntDisable
(
    IN GT_U32   intVecNum
)
{
  SHOSTG_interrupt_disable_one(intVecNum);

  return GT_OK ;
}

/**
* @internal SHOSTG_interrupt_do_enable function
* @endinternal
*
* @brief   Lock/unlock interrupts
*/
extern void SHOSTG_interrupt_do_enable(UINT_32 new_val);
GT_32 osSetIntLockUnlock
(
    IN      INTERRUPT_MODE mode,
    INOUT   GT_32          *key
)
{
    static INTERRUPT_MODE currMode = INTR_MODE_UNLOCK;
    static int intLockCnt = 0;
    INTERRUPT_MODE temp;
  
    /* lock locking counter */
    osMutexLock(IntLockmutex, 0);

    if (mode == INTR_MODE_LOCK)
        intLockCnt++;
    else if ((mode == INTR_MODE_UNLOCK) && (intLockCnt > 0))
        intLockCnt--;

    temp = currMode;
    currMode = mode;

    if ((mode == INTR_MODE_UNLOCK) && (intLockCnt == 0))
    {
        osMutexUnlock(IntLockmutex);
        if (osInterGo == 0)
        {
            SHOSTG_go_intr_go ();
            osInterGo = 1;
        }

        SHOSTG_interrupt_do_enable(*key);

    }
    else
    {

        osMutexUnlock(IntLockmutex);
    }

    return temp;
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
    GT_U32  shostIntVector = vector + 1;
    /* call SHOST to bind ISR with simulated interrupts
       engine */
    SHOSTG_bind_interrupt_with_param ( 15, shostIntVector,
                                       routine, NULL, parameter);

    return GT_OK;
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
