/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
#include <cmdShell/os/cmdOs.h>
#include <bootLib.h>
#include <string.h>

extern BOOT_PARAMS	sysBootParams;

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
    const char *t;
    /* sysBootParams.had = 172.22.97.5 */
    if (sysBootParams.had[0] == 0)
        return GT_FAIL;
    strcpy(server_ip, sysBootParams.had);
    local_ip[0] = 0;
    /* sysBootParams.ead = 172.22.97.8:FFFF0000 */
    if ((t = strchr(sysBootParams.ead, ':')) != NULL)
    {
        strncpy(local_ip, sysBootParams.ead, t - sysBootParams.ead);
        local_ip[t - sysBootParams.ead] = 0;
    }
    else
    {
        if (strlen(sysBootParams.ead) < 16)
        {
            strcpy(local_ip, sysBootParams.ead);
        }
    }

    return GT_OK;
}


