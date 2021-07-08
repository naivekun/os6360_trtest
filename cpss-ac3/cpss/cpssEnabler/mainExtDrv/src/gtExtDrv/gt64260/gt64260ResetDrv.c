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
* @file gt64260ResetDrv.c
*
* @brief Includes two function for reset of CPU.
*
* @version   1.1.2.1
********************************************************************************
*/

#include <gtExtDrv/drivers/gtPciDrv.h>
#include <coreDrv/gtCore.h>

#include <sysLib.h>

/**
* @internal extDrvResetInit function
* @endinternal
*
* @brief   This routine calls in init to do system init config for reset.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvResetInit
(
)
{
    GT_U32 data;

    GT_REG_READ(0xf00c, &data);
    data &= 0xfffffff0;
    GT_REG_WRITE(0xf00c, data);
    
    GT_REG_READ(0xf100, &data);
    data &= ~(1<<24);
    GT_REG_WRITE(0xf100, data);
    
    return GT_OK;
}

/**
* @internal extDrvReset function
* @endinternal
*
* @brief   This routine calls to reset of CPU.
*
* @retval GT_FAIL                  - always.
*/
GT_STATUS extDrvReset
(
)
{
    GT_U32 data;
    
    GT_REG_READ(0xf100, &data);
    data |= 1<<24;
    GT_REG_WRITE(0xf100, data);
    
    GT_REG_READ(0xf104, &data);
    data |= 1<<24;
    GT_REG_WRITE(0xf104, data);

    /*Reset*/
    GT_REG_READ(0xf104, &data);
    data &= ~(1<<24);
    GT_REG_WRITE(0xf104, data);
    
    osTimerWkAfter(100);

    GT_REG_READ(0xf104, &data);
    data |= (1<<24);
    GT_REG_WRITE(0xf104, data);
    
    /* Reset the board from the BSP sysToMonitor function */
    sysToMonitor(0);
    while(1) data++;

    return GT_FAIL;
}




