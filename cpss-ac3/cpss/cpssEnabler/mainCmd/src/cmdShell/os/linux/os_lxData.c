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
* @file os_lxData.c
*
* @brief This file contains OS info functions
*
* @version   2
********************************************************************************
*/
#include <cmdShell/os/cmdOs.h>
#include <string.h>
#include <stdio.h>


/**
* @internal prvCmdOsGetServerAndLocalIPIntelCPU function
* @endinternal
*
* @brief   Returns TFTP server IP address and local IP from
*         /boot/persist/mv_env/serverip and /boot/persist/mv_env/ipaddress
*
* @param[out] ip
* @param[out] ip
*/
GT_STATUS prvCmdOsGetServerAndLocalIPIntelCPU(
  IN GT_CHAR *path,
  OUT GT_CHAR *ip
){
    FILE    *fp;
    char    cmdLineBuf[1024];
    char    *cmdLineTmp = NULL;
    int     numberOfBytesTmp;
    int     numberOfBytesToCopy;
    fp = fopen(path, "r");
    if (fp == NULL)
    {
        return GT_FAIL;
    }
    if (fgets(cmdLineBuf,sizeof(cmdLineBuf),fp) == NULL) {
        cmdLineBuf[0] = 0;
    }
    fclose(fp);
    /* lookup for "=<ip> */
    cmdLineTmp = strstr(cmdLineBuf,"=");
    if (cmdLineTmp == NULL) {
        return GT_FAIL; 
    }
	
    cmdLineTmp += 1;
    numberOfBytesTmp = cmdLineTmp - cmdLineBuf;
    numberOfBytesToCopy = strlen(cmdLineBuf) - numberOfBytesTmp - 1;
    strncpy(ip, cmdLineTmp, numberOfBytesToCopy);
    ip[numberOfBytesToCopy] = 0;  
    return GT_OK;
}

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
#ifndef ASIC_SIMULATION
    FILE    *fp;
    char    cmdLineBuf[1024];
    char    *cmdLineTmp, *t;
    GT_STATUS rc;
    
    fp = fopen("/proc/cmdline", "r");
    if (fp == NULL)
    {
        return GT_FAIL;
    }
    if (fgets(cmdLineBuf,sizeof(cmdLineBuf),fp) == NULL) {
        cmdLineBuf[0] = 0;
    }
    fclose(fp);
    /* lookup for   ip=<localIp>:<serverIp>:... */
    cmdLineTmp = strstr(cmdLineBuf,"ip=");
    
    if (cmdLineTmp == NULL) {
        /* in INTEL CPU, the ip is saved elsewhere, therefor, if "ip=" wasn't found here
           then it will be searched in another file*/
        rc = prvCmdOsGetServerAndLocalIPIntelCPU("/boot/persist/mv_env/ipaddress", local_ip);
        if (rc == GT_FAIL) {
            return rc;
        }
        rc = prvCmdOsGetServerAndLocalIPIntelCPU("/boot/persist/mv_env/serverip", server_ip);
        if (rc == GT_FAIL) {
            return rc;
        }
        return GT_OK;
    }
    cmdLineTmp += 3;
    if ((t=strchr(cmdLineTmp,':'))==NULL)
    {
        return GT_FAIL;
    }
    if ((t == cmdLineTmp) || (t - cmdLineTmp > 15))
    {
        return GT_FAIL;
    }
    strncpy(local_ip, cmdLineTmp, t - cmdLineTmp);
    local_ip[t - cmdLineTmp] = 0;
    cmdLineTmp = t + 1;

    if ((t=strchr(cmdLineTmp,':'))==NULL)
    {
        return GT_FAIL;
    }
    if ((t == cmdLineTmp) || (t - cmdLineTmp > 15))
    {
        return GT_FAIL;
    }
    strncpy(server_ip, cmdLineTmp, t - cmdLineTmp);
    server_ip[t - cmdLineTmp] = 0;
    return GT_OK;

#else /* defined(ASIC_SIMULATION) */
    (void)local_ip;
    (void)server_ip;
    return GT_FAIL;
#endif /* defined(ASIC_SIMULATION) */
}


