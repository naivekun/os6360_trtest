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
* @file os_w32Data.c
*
* @brief This file contains OS info functions
*
* @version   1
********************************************************************************
*/
#include <cmdShell/os/cmdOs.h>

/**
* @internal cmdOsGetServerAndLocalIP function
* @endinternal
*
* @brief   Returns TFTP server IP address and local IP
*
* @param[out] local_ip
* @param[out] server_ip
*/
GT_STATUS cmdOsGetServerAndLocalIP(
  OUT GT_CHAR *local_ip,
  OUT GT_CHAR *server_ip
)
{
    return GT_FAIL;
}


