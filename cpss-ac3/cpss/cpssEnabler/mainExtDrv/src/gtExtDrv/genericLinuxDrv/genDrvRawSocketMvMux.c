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
* @file genDrvRawSocketMvMux.c
*
* @brief This file includes implementation for raw socket support on
* AXP, MSYS (mv_mux driver)
*
* @version   1
********************************************************************************
*/
#define  _SVID_SOURCE
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h> /* ntohs */
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include <cpss/common/cpssTypes.h>
#include <gtExtDrv/drivers/gtEthPortCtrl.h>
#include <gtOs/gtOs.h>
#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsTask.h>
#include "kerneldrv/include/presteraSmiGlob.h"



extern extDrvEthTxMode_ENT gTxMode;
extern GT_RawSocketRx_FUNCPTR userRawSocketRxCbFunc;
extern GT_Tx_COMPLETE_FUNCPTR userTxCbFunc;

static GT_STATUS initialized = GT_NOT_INITIALIZED;
static GT_TASK rawSocketTid;

#define SOCK_PROTO_MV   0x1234

#define MAXPACKET 2048
#define IFNAME_PREFIX "mux"
static int sockFd = -1;

/* printf(fmt,p1,p2) > /sys/devices/platform/mv_mux/${f} */
static GT_STATUS prvExtDrvMvMuxCmd(const char *f, const char *fmt, int p1, int p2)
{
    char buf[128];
    int fd;
    sprintf(buf, "/sys/devices/platform/mv_mux/%s",f);
    fd = open(buf, O_WRONLY);
    if (fd < 0)
        return GT_FALSE;
    sprintf(buf, fmt, p1, p2);
    write(fd, buf, strlen(buf));
    close(fd);
    return GT_OK;
}

/**
* @internal prvExtDrvMvMuxIfconfigUpPromisc function
* @endinternal
*
* @brief   ifconfig $(ifname) up running promisc
*         doesn't require external 'ifconfig'
*         will work when no nfs available
*
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvExtDrvMvMuxIfconfigUpPromisc(const char *ifname)
{
    int fd;
    struct ifreq ifr;

    fd = socket(PF_PACKET, SOCK_RAW, 0);
    if (fd < 0)
    {
        perror("socket()");
        return GT_FAIL;
    }
    strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0)
    {
        perror("ioctl(sockfd, SIOCGIFFLAGS, &ifr");
        close(fd);
        return GT_FAIL;
    }

    ifr.ifr_flags |= IFF_UP | IFF_RUNNING | IFF_PROMISC;
    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0)
    {
        perror("ioctl(sockfd, SIOCSIFFLAGS, &ifr");
        close(fd);
        return GT_FAIL;
    }
    close(fd);
    return GT_OK;
}

/**
* @internal prvExtDrvMvMuxConfigure function
* @endinternal
*
* @brief   configure mux$(muxPort) interface with eth$(portNum) interface
*
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvExtDrvMvMuxConfigure(int portNum, int muxPort)
{
    char ifname[32];
    prvExtDrvMvMuxCmd("leave_tag",  "mux%d 1", portNum, 0);
    prvExtDrvMvMuxCmd("proto_type", "mux%d 0x%x", portNum, SOCK_PROTO_MV);
    prvExtDrvMvMuxCmd("tag_type",   "%d 2", muxPort, 0);
    prvExtDrvMvMuxCmd("add",        "mux%d %d", portNum, muxPort);

#if 0
    prvExtDrvMvMuxCmd("dump", "%d",0,0);
#endif
    sprintf(ifname, "eth%d", portNum);
    prvExtDrvMvMuxIfconfigUpPromisc(ifname);
    sprintf(ifname, "mux%d", muxPort);
    prvExtDrvMvMuxIfconfigUpPromisc(ifname);

    return GT_OK;
}

/**
* @internal prvExtDrvMvMuxConfig function
* @endinternal
*
* @brief   configure all mv_mux driver and mux interface
*
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvExtDrvMvMuxConfig(void)
{
    printf("=== configure mv_mux for Raw Socket ===\n");
    /* to don't mix message with kernel messages
     * flush sdtout then wait till all output written */
    fflush(stdout);
    tcdrain(1);
    prvExtDrvMvMuxConfigure(0,0);
    sleep(1);
    printf("=== mv_mux configured ===\n");
    fflush(stdout);
    tcdrain(1);

    return GT_OK;
}

/**
* @internal prvExtDrvMvMuxOpenRawSocket function
* @endinternal
*
* @brief   Open raw socket for Rx/Tx
*
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvExtDrvMvMuxOpenRawSocket(void)
{
    int fd;
    struct ifreq ifr;
    struct sockaddr_ll sa;

    fd = socket(PF_PACKET, SOCK_RAW, htons(SOCK_PROTO_MV));

    if (fd < 0)
    {
        perror("rx socket error");
        return GT_FAIL;
    }

    strncpy(ifr.ifr_name, "mux0", sizeof(ifr.ifr_name));

    /* get the index */
    if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0)
    {
        perror("ioctl: SIOCGIFINDEX");
        close(fd);
        return GT_FAIL;
    }

    /* Bind to the interface name */
    memset(&sa, 0, sizeof(sa));
    sa.sll_family = AF_PACKET;
    sa.sll_ifindex = ifr.ifr_ifindex;
    sa.sll_protocol = htons(SOCK_PROTO_MV);

    if (bind(fd, (struct sockaddr *)&sa, sizeof(sa)))
    {
        perror("bind error");
        close(fd);
        return GT_FAIL;
    }

    sockFd = fd;

    return GT_OK;
}

/**
* @internal prvExtDrvSendRawSocket function
* @endinternal
*
* @brief   This function transmits a packet through the linux raw socket
*
* @param[in] segmentList[]            - A list of pointers to the packets segments.
* @param[in] segmentLen[]             - A list of segement length.
* @param[in] numOfSegments            - The number of segment in segment list.
* @param[in] txQueue                  - The TX queue.
*
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvExtDrvSendRawSocket
(
    IN GT_U8_PTR        segmentList[],
    IN GT_U32           segmentLen[],
    IN GT_U32           numOfSegments,
    IN GT_U32           txQueue
)
{
    int i;
    int len = 0;
    char *packet;
    char *p;

    if (sockFd < 0)
        return GT_OK;

    for (i = 0; i < numOfSegments; i++)
        len += segmentLen[i];

    if (len == 0)
        return GT_OK;

    p = packet = (char *)malloc(len);

    for (i = 0; i < numOfSegments; i++)
    {
        memcpy(p, segmentList[i], segmentLen[i]);
        p += segmentLen[i];
    }

    if(sendto(sockFd, packet, len, 0,NULL, 0) < 0)
    {
        perror("sendto() error");
    }

    if ((gTxMode == extDrvEthTxMode_synch_E) && (userTxCbFunc != NULL))
        userTxCbFunc(segmentList, numOfSegments);

    free(packet);
    return GT_OK;
}


/*******************************************************************************
* rawSocketTask
*
* DESCRIPTION:
*       Raw socker Rx handler task.
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
static unsigned __TASKCONV rawSocketTask(GT_VOID* unused)
{
    int                 k;
    char                *packet = NULL;

    for ( ; ; )
    {
        if (packet == NULL)
        {
            packet = (char *)osMalloc(MAXPACKET+4);
            /* printf(">>>> malloced packet=%p\n", packet); */
        }
        k = recv(sockFd, packet+4, MAXPACKET, 0);
        if ((k > 0) && (userRawSocketRxCbFunc != NULL))
        {
            /* all packets arriving here have the full dsa tag.
               port number is not required here */
            userRawSocketRxCbFunc((GT_U8_PTR)(packet+4), k, 0);
            packet = NULL; /* allocate new buffer */
        }
    }
    /* no return */
    return 0;
}

/**
* @internal extDrvEthMuxSet function
* @endinternal
*
* @brief   Sets the mux mode to one of cpss, raw, linux
*/
GT_STATUS extDrvEthMuxSet
(
 IN unsigned long portNum,
 IN extDrvEthNetPortType_ENT portType
)
{
    return GT_OK;
}

/**
* @internal extDrvEthMuxGet function
* @endinternal
*
* @brief   Get the mux mosde of the port
*/
GT_STATUS extDrvEthMuxGet
(
 IN unsigned long portNum,
 OUT extDrvEthNetPortType_ENT *portTypeP
)
{
    *portTypeP = bspEthNetPortType_raw_E;
    return GT_OK;
}

/**
* @internal prvExtDrvRawSocketEnable function
* @endinternal
*
* @brief   Enable raw socket interface
*/
GT_STATUS prvExtDrvRawSocketEnable(GT_VOID)
{
    return GT_OK;
}

/**
* @internal extDrvLinuxModeSet function
* @endinternal
*
* @brief   Set port <portNum> to Linux Mode (Linux Only)
*
* @param[in] portNum                  - The port number to be defined for Linux mode
* @param[in] ip1                      ip2, ip3, ip4 - The ip address to assign to the port, 4 numbers
*
* @retval GT_OK                    - always,
*/
GT_STATUS extDrvLinuxModeSet
(
 IN GT_U32 portNum,
 IN GT_U32 ip1,
 IN GT_U32 ip2,
 IN GT_U32 ip3,
 IN GT_U32 ip4
)
{
    return GT_NOT_SUPPORTED;
}

/**
* @internal prvExtDrvRawSocketInit function
* @endinternal
*
* @brief   Initialize raw socket operations
*
* @retval GT_OK                    - if success
*/
GT_STATUS prvExtDrvRawSocketInit(GT_VOID)
{
    GT_STATUS rc;
    /* open all active nics */
    if (initialized != GT_NOT_INITIALIZED)
        return initialized;

    prvExtDrvMvMuxConfig();

    initialized = prvExtDrvMvMuxOpenRawSocket();
    if (initialized != GT_OK)
    {
        return GT_OK; /* initialized??? */
    }

    rc = osTaskCreate("rawSocketTask", 0, 0xa000, rawSocketTask , NULL, &rawSocketTid);

    if (rc != GT_OK)
    {
        fprintf(stderr, "prvExtDrvRawSocketInit: osTaskCreate(): %d\n", rc);
        return GT_FAIL;
    }
    return GT_OK;
}


