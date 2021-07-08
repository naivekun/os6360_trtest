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
* @file osLinuxIntr.c
*
* @brief Linux User Mode Operating System wrapper. Interrupt facility.
*
* @version   12
********************************************************************************
*/
#ifdef LINUX_SIM

#include <gtOs/gtOsIntr.h>
#include <gtOs/gtOsSem.h>
#include <gtOs/gtOsSharedData.h>

#include <asicSimulation/SCIB/scib.h>
#include <common/SHOST/GEN/INTR/EXP/INTR.H>

/***** Public Functions ************************************************/

/******Local Variables ************************/
GT_MUTEX IntLockmutex __SHARED_DATA_MAINOS = (GT_MUTEX)0;

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

/*******************************************************************************
* osSetIntLockUnlock
*
* DESCRIPTION:
*       Lock/unlock interrupts
*
* INPUTS:
*       mode   - interrupt state lock/unlock
*       key    - if mode is INTR_MODE_UNLOCK, lock key returned by
*                preceding interrupt disable call
*
* OUTPUTS:
*       key    - if mode is INTR_MODE_LOCK lock key for the interrupt
*                level
*
* RETURNS:
*       Lock key for the interrupt level
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_32 osSetIntLockUnlock
(
    IN      INTERRUPT_MODE mode,
    INOUT   GT_32          *key
)
{
    GT_32 retValue = 0;
	static GT_U32 numOfNotInitLockings = 0;
    (void)key;

    if (mode == INTR_MODE_LOCK)
    {
        if(IntLockmutex)
        {
            osMutexLock(IntLockmutex);
        }
        else
        {
            numOfNotInitLockings++;
        }
    }
    else
    {
        if(IntLockmutex)
        {
            osMutexUnlock(IntLockmutex);
        }
        else
        {
            numOfNotInitLockings--;
        }
    }

    return retValue;
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
    IN  GT_U32           vector
)
{
    GT_U32  shostIntVector = vector + 1;
    /* call SHOST to unbind ISR */
    SHOSTG_unbind_interrupt_with_param (shostIntVector);

    return GT_OK;
}
#else /* !LINUX_SIM */

#include <gtOs/gtOsIntr.h>

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
    if (mode == INTR_MODE_LOCK)
        *key = 0;
    return GT_OK;
}
#endif /*LINUX_SIM*/



