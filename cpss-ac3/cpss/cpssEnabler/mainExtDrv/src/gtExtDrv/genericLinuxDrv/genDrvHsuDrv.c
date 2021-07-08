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
* @file genDrvHsuDrv.c
*
* @brief Includes HSU function wrappers, for HSU functionality
*
* @version   6
********************************************************************************
*/
#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <gtExtDrv/drivers/gtDragoniteDrv.h>
#include <gtExtDrv/drivers/pssBspApis.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>
#include <sys/mman.h>

#include <sys/ioctl.h>
#include "kerneldrv/include/presteraGlob.h"
extern GT_32 gtPpFd;                /* pp file descriptor           */

#if defined(GDA8548_DRV) || defined(XCAT_DRV) || defined(INTEL64_DRV)
static GT_UINTPTR hsuBaseAddr = 0;
#endif

/**
* @internal extDrvHsuMemBaseAddrGet function
* @endinternal
*
* @brief   Get start address of HSU area
*
* @param[out] hsuPtrAddr               - Pointer address to beginning of HSU area
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS extDrvHsuMemBaseAddrGet
(
    OUT  GT_U32  **hsuPtrAddr
)
{
#if defined(GDA8548_DRV) || defined(XCAT_DRV) || defined(INTEL64_DRV)
  if (hsuBaseAddr == 0)
  {
    struct GT_PCI_VMA_ADDRESSES_STC vmConfig;

    hsuBaseAddr = LINUX_VMA_HSU;
    if (ioctl(gtPpFd, PRESTERA_IOC_GETVMA, &vmConfig) == 0)
    {
        hsuBaseAddr = (GT_UINTPTR)vmConfig.hsuBaseAddr;
    }
  }

  *hsuPtrAddr = (GT_U32*)hsuBaseAddr;
#endif

  return GT_OK;
}

/**
* @internal extDrvHsuWarmRestart function
* @endinternal
*
* @brief   Performs warm restart of the 8548 cpu
*/

GT_VOID extDrvHsuWarmRestart
(
    GT_VOID
)
{
#if defined(GDA8548_DRV) || defined(XCAT_DRV) || defined(INTEL64_DRV)
    if (ioctl(gtPpFd, PRESTERA_IOC_HSUWARMRESTART) < 0)
    fprintf(stderr, "Fail to perform Hsu warm restart\n");
#endif
}

/**
* @internal extDrvHsuInboundSdmaEnable function
* @endinternal
*
* @brief   This routine enables cpu inbound sdma access .
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvHsuInboundSdmaEnable
(
    GT_VOID
)
{
    /* stub */
    /* do nothing */
    return GT_OK;
}

/**
* @internal extDrvHsuInboundSdmaDisable function
* @endinternal
*
* @brief   This routine disables cpu inbound sdma access .
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvHsuInboundSdmaDisable
(
    GT_VOID
)
{
    /* stub */
    /* do nothing */
    return GT_OK;
}

/**
* @internal extDrvHsuInboundSdmaStateGet function
* @endinternal
*
* @brief   This routine gets the state of cpu inbound sdma access.
*
* @param[out] enablePtr                - (pointer to) inbound sdma status .
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvHsuInboundSdmaStateGet
(
    OUT GT_BOOL *enablePtr
)
{
    /* stub */
    /* do nothing */
    *enablePtr = GT_FALSE;
    return GT_NOT_SUPPORTED;
}



