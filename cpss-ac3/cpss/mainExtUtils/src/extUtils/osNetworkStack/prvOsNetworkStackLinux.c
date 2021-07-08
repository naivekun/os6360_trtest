/*******************************************************************************
*                Copyright 2014, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file prvOsNetworkStackLinux.c
*
* @brief OS TCP/IP stack API
* Linux BM implementation
*
* @version   2
********************************************************************************
*/
#ifndef LINUX
# error "Linux only"
#endif
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_tun.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

/* interface file descriptor */
static int tnFd = -1;
/* interface name */
static char tnName[IFNAMSIZ];

/* forward declaration */
GT_STATUS prvOsNetworkStackIfconfig(
  IN const char *config
);

/**
* @internal prvOsNetworkStackDeviceOpen function
* @endinternal
*
* @brief   Open and initialize OS network device (usually tap0)
*/
GT_STATUS prvOsNetworkStackDeviceOpen(void)
{
    struct ifreq ifr;
    if (tnFd >= 0)
        return GT_OK;
    
    /* open the clone device */
    if ( (tnFd = open("/dev/net/tun", O_RDWR)) < 0 )
    {
        cpssOsPrintf("prvOsNetworkStackDeviceOpen(): "
                    "Failed to open /dev/net/tun: %s\r\n",
                    strerror(errno));
        return GT_FAIL;
    }

    /* preparation of the struct ifr, of type "struct ifreq" */
    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    /* the kernel will try to allocate the "next" device of the TAP type */

    /* try to create the device */
    if (ioctl(tnFd, TUNSETIFF, (void *) &ifr) < 0)
    {
        cpssOsPrintf("prvOsNetworkStackDeviceOpen(): "
                    "Failed to clone TAP device (TUNSETIFF): %s\r\n",
                    strerror(errno));
        close(tnFd);
        tnFd = -1;
        return GT_FAIL;
    }

    /* if the operation was successful, write back the name of the
     * interface to the variable tnName, so we will know it
     */
    strcpy(tnName, ifr.ifr_name);

    prvOsNetworkStackIfconfig("mtu 1500");
    return GT_OK;
}

/**
* @internal prvOsNetworkStackDeviceClose function
* @endinternal
*
* @brief   Close OS network device (usually tap0)
*/
GT_STATUS prvOsNetworkStackDeviceClose(void)
{
    if (tnFd < 0)
        return GT_OK;
    prvOsNetworkStackIfconfig("down");
    close(tnFd);
    tnFd = -1;
    return GT_OK;
}

/**
* @internal prvOsNetworkStackDeviceRead function
* @endinternal
*
* @brief   Read packet from OS netwrok stack
*
* @param[in,out] pktLen                   - length of pktBuf
*
* @param[out] pktBuf                   - pointer for packet data
* @param[in,out] pktLen                   - actual length of packet data
*                                       GT_STATUS
*/
GT_STATUS prvOsNetworkStackDeviceRead(
   OUT   GT_U8  *pktBuf,
   INOUT GT_U32 *pktLen
)
{
    int nread;
    if (tnFd < 0)
        return GT_NOT_INITIALIZED;
    while(1)
    {
        nread = read(tnFd, pktBuf, (size_t)(*pktLen));
        if (nread > 0)
            break;
        if (errno != EINTR)
            return GT_FAIL;
    }
    *pktLen = (GT_U32)nread;
    return GT_OK;
}

/**
* @internal prvOsNetworkStackDeviceWrite function
* @endinternal
*
* @brief   Write packet to OS netwrok stack
*
* @param[in] pktBuf                   - pointer for packet data
* @param[in] pktLen                   - length of pktBuf
*                                       GT_STATUS
*/
GT_STATUS prvOsNetworkStackDeviceWrite(
  IN    GT_U8   *pktBuf,
  IN    GT_U32  pktLen
)
{
    int nwrite;
    if (tnFd < 0)
        return GT_NOT_INITIALIZED;
    nwrite = write(tnFd, pktBuf, (size_t)pktLen);
    return (nwrite < 0) ? GT_FAIL : GT_OK;
}

/**
* @internal prvOsNetworkStackMacGet function
* @endinternal
*
* @brief   Get mac address of OS network device (usually tap0)
*
* @param[out] etherPtr                 - pointer to store mac address
*                                       GT_STATUS
*/
GT_STATUS prvOsNetworkStackMacGet(
  OUT GT_ETHERADDR *etherPtr
)
{
    struct ifreq ifr;

    if (tnFd < 0)
        return GT_NOT_INITIALIZED;

    memset(&ifr, 0, sizeof(ifr));
    if (ioctl(tnFd, SIOCGIFHWADDR, (void *) &ifr) < 0)
    {
        return GT_FAIL;
    }
    memcpy(etherPtr->arEther, ifr.ifr_hwaddr.sa_data, 6);
    return GT_OK;
}

/**
* @internal prvOsNetworkStackMacSet function
* @endinternal
*
* @brief   Get mac address of OS network device (usually tap0)
*
* @param[in] etherPtr                 - mac address to set
*                                       GT_STATUS
*/
GT_STATUS prvOsNetworkStackMacSet(
  IN  GT_ETHERADDR *etherPtr
)
{
    struct ifreq ifr;

    if (tnFd < 0)
        return GT_NOT_INITIALIZED;
    
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, tnName);
    memcpy(ifr.ifr_hwaddr.sa_data, etherPtr->arEther, 6);
    ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;

    if (ioctl(tnFd, SIOCSIFHWADDR, (void *) &ifr) < 0)
    {
        return GT_FAIL;
    }
    return GT_OK;

}

/**
* @internal prvOsNetworkStackIfconfig function
* @endinternal
*
* @brief   Configure OS network interface
*
* @param[in] config                   - Configuration string
*                                       GT_STATUS
*
* @note Example:
*       prvOsNetworkStackIfconfig("up inet 10.1.2.3 netmask 255.255.255.0")
*
*/
GT_STATUS prvOsNetworkStackIfconfig(
  IN const char *config
)
{
    char cmd[BUFSIZ];

    if (BUFSIZ <= (strlen("ifconfig ") + strlen(tnName) + strlen(config)))
    {
        return GT_BAD_PARAM;
    }
    cpssOsSprintf(cmd, "ifconfig %s %s", tnName, config); 
#define ignore_unused_result(_cmd) { int rc = _cmd; (void)sizeof rc; }
    ignore_unused_result(system(cmd));
    return GT_OK;
}


