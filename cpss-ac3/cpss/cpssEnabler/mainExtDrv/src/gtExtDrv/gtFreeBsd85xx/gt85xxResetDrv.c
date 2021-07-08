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
* @file gt85xxResetDrv.c
*
* @brief Includes two function for reset of CPU.
*
* @version   1
********************************************************************************
*/

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <gtExtDrv/drivers/gtPciDrv.h>
#include "kerneldrv/include/prestera_glob.h"

/* Globals */

/* file descriptor returnd by openning the PP *nix device driver */
extern GT_32 gtPpFd;

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
    if (ioctl (gtPpFd, PRESTERA_IOC_HWRESET_INIT))
    {
        fprintf(stderr, "extDrvResetInit failed: errno(%s)\n",
                        strerror(errno));
        return GT_FAIL;
    }

    return GT_OK;
}

/**
* @internal extDrvReset function
* @endinternal
*
* @brief   This routine calls to reset of CPU.
*
* @retval GT_FAIL                  - always, I keep this like in Linux, but = 1 i.e.
* @retval non                      -zero value, hopefully it won't cause prolems in
*                                       FreeBSD too.
*/
GT_STATUS extDrvReset
(
)
{
    if (ioctl (gtPpFd, PRESTERA_IOC_HWRESET))
    {
        fprintf(stderr, "extDrvReset failed: errno(%s)\n",
                        strerror(errno));
    }

    return GT_FAIL;
}




