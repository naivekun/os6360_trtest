/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
#include <gtExtDrv/drivers/gtIntDrv.h>
#include <gtExtDrv/drivers/pssBspApis.h>
/**
* @internal extDrvIntConnect function
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
GT_STATUS extDrvIntConnect
(
    IN  GT_U32           vector,
    IN  GT_VOIDFUNCPTR   routine,
    IN  GT_U32           parameter
)
{
   return osInterruptConnect(vector, routine, parameter);
}

/**
* @internal extDrvIntEnable function
* @endinternal
*
* @brief   Enable corresponding interrupt bits
*
* @param[in] intMask                  - new interrupt bits
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS extDrvIntEnable
(
    IN GT_U32   intMask
)
{
   return osIntEnable(intMask);
}

/**
* @internal extDrvIntDisable function
* @endinternal
*
* @brief   Disable corresponding interrupt bits.
*
* @param[in] intMask                  - new interrupt bits
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS extDrvIntDisable
(
    IN GT_U32   intMask
)
{
   return osIntDisable(intMask);
}

/**
* @internal extDrvSetIntLockUnlock function
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
GT_32 extDrvSetIntLockUnlock
(
    IN      INTERRUPT_MODE mode,
    INOUT   GT_32          *key
)
{
    return osSetIntLockUnlock(mode, key);
}


