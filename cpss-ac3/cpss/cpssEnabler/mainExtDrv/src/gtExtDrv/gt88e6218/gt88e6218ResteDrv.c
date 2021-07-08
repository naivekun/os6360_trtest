/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

#include <gtExtDrv/drivers/gtPciDrv.h>
#include <gtExtDrv/drivers/gtSmiHwCtrl.h>

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
    /* #warning "extDrvResetInit not yet implemented" */
    /* Soft rest trigger */
    hwIfSmiTaskWriteReg(0, 0x30, 1 << 16);
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
#if !(defined (ASIC_SIMULATION))
    /* #warning "extDrvResetInit not yet implemented use reboot for now" */
    extern void reboot(int);
    reboot(2); /* vxWorks reboot with clear */
#else
    extern void exit(int);
    exit(0);
#endif

    return GT_OK;
}



