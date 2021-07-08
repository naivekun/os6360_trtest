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
* @file noKmDrvResetDrv.c
*
* @brief Includes two function for reset of CPU.
*
* @version   1
********************************************************************************
*/
#include <gtExtDrv/drivers/gtPciDrv.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/reboot.h>
#include <private/8245/gtCore.h>


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
    GT_VOID
)
{
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
 GT_VOID
)
{
  if (system("reboot") != 0)
  {
      fprintf(stderr, "\r\n\n*** Failed to while executing system(\"reboot\") ***\r\n");
  }
  return GT_FAIL;
}




