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
* @file gtOsIntr.h
*
* @brief Operating System wrapper. Interrupt facility.
*
* @version   8
********************************************************************************
*/

#ifndef __gtOsIntrh
#define __gtOsIntrh

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes *******************************************************/

#include <gtOs/gtGenTypes.h>


/************* Typedefs *******************************************************/
typedef enum
{
    INTR_MODE_LOCK =0,
    INTR_MODE_UNLOCK
}INTERRUPT_MODE;


/************* Functions ******************************************************/
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
);

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
);

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
);

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
);

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
);

#ifdef __cplusplus
}
#endif

#endif  /* __gtOsIntrh */
/* Do Not Add Anything Below This Line */



