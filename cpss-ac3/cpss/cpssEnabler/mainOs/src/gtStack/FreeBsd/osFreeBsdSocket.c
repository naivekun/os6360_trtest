/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>

#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsMem.h>
#include <gtOs/gtGenTypes.h>
#include <gtStack/gtStackTypes.h>
#include <gtStack/gtOsSocket.h>
#include <gtUtil/gtBmPool.h>

#define SOCKET_ADDR_POOL_SIZE   100

/* pool ID to be used for allocating    */
/* memory for socket address structures */
GT_POOL_ID  socketAddrPool = 0;

/**
* @internal osSocketLastError function
* @endinternal
*
* @brief   Rerturn last error code for a given socket
*
* @param[in] socketFd                 - Socket descriptor
*
* @retval 0                        - no error
*                                       GT_SOCKET_E on error
*/
GT_STATUS osSocketLastError
(
    IN  GT_SOCKET_FD   socketFd
)
{
    int i, error = 0;
    socklen_t len = sizeof(error);
    struct {
        int errno_code;
        int err;
    } err_map[] = {
        { EAGAIN, GT_SOCKET_EAGAIN },
        { EINTR, GT_SOCKET_EINTR },
        { EINVAL, GT_SOCKET_EINVAL },
        { EMSGSIZE, GT_SOCKET_EMSGSIZE },
        { EISCONN, GT_SOCKET_EISCONN },
        { ESHUTDOWN, GT_SOCKET_SHUTDOWN },
        { EWOULDBLOCK, GT_SOCKET_EWOULDBLOCK },
        { EINPROGRESS, GT_SOCKET_EINPROGRESS },
        { EALREADY, GT_SOCKET_EALREADY },
        { ECONNRESET, GT_SOCKET_ECONNRESET },
        { ECONNREFUSED, GT_SOCKET_ECONNREFUSED },
        { EBADF, GT_SOCKET_EBADF },
        { ENETUNREACH, GT_SOCKET_ENETUNREACH },
        { EHOSTUNREACH, GT_SOCKET_EHOSTUNREACH },
        { 0, 0 }
    };
    i = getsockopt(socketFd, SOL_SOCKET, SO_ERROR, (void *)&error, &len );
    if (i != 0)
        return GT_SOCKET_ERROR_BASE;
    if (error == 0)
        return GT_OK;
    for (i = 0; err_map[i].errno_code != 0; i++)
    {
        if (err_map[i].errno_code == error)
            return err_map[i].err;
    }
    return GT_SOCKET_ERROR_BASE;
}

/*******************************************************************************
* osSocketLastErrorStr
*
* DESCRIPTION:
*       returns error string for last error
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       string
*
* COMMENTS:
*       None
*
*******************************************************************************/
const char * osSocketLastErrorStr(void)
{
    return strerror(errno);
}

/**
* @internal osSocketTcpCreate function
* @endinternal
*
* @brief   Create a socket from TCP type
*/
GT_SOCKET_FD osSocketTcpCreate (GT_32 sockSize)
{
    GT_SOCKET_FD sock;
    int          optval = 1;
    
    sock = socket(AF_INET, SOCK_STREAM, 0) ;
    
    if (sock < 0)
    {
        return GT_ERROR;
    }
    
    /* if REUSEADDR and REUSEPORT not defined and program closed and opend 
     * during short time OS returns error - "address already in use"
     */
    if(setsockopt(sock, SOL_SOCKET,  SO_REUSEADDR, (GT_CHAR*) &optval, sizeof(optval)) != 0)
    {
        return GT_ERROR;
    }
    if(setsockopt(sock, SOL_SOCKET,  SO_REUSEPORT, (GT_CHAR*) &optval, sizeof(optval)) != 0)
    {
        return GT_ERROR;
    }
    /* TCP_NODELAY option needed to increase performance - it says to socket 
     * don't wait to buffer full - send info immediately
     */
    if(setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,  (GT_CHAR*) &optval, sizeof(optval)) != 0)
    {
        return GT_ERROR;
    }
    
    if(sockSize != GT_SOCKET_DEFAULT_SIZE)
    {   
        optval = sockSize;

        if (setsockopt (sock, SOL_SOCKET, SO_SNDBUF, (GT_CHAR*)&optval,
                        sizeof (optval)) != 0)
        {
            return GT_ERROR;
        }
        if (setsockopt (sock, SOL_SOCKET, SO_RCVBUF, (GT_CHAR*)&optval,
                        sizeof (optval)) != 0)
        {
            return GT_ERROR;
        }
    }

    return sock;
}


/**
* @internal osSocketUdpCreate function
* @endinternal
*
* @brief   Create a socket from UDP type
*/
GT_SOCKET_FD osSocketUdpCreate (GT_32 sockSize)
{
    GT_SOCKET_FD sock;
    int          optval;
    
    sock = socket(AF_INET, SOCK_DGRAM, 0) ;

    if (sock < 0)
    {
        return GT_ERROR;
    }    
    /* setsockopt(sock, SOL_SOCKET,  SO_REUSEADDR, (GT_CHAR*) &optval, sizeof(optval)); */

    if(sockSize != GT_SOCKET_DEFAULT_SIZE)
    {   
        optval = sockSize;

        if (setsockopt (sock, SOL_SOCKET, SO_SNDBUF, (GT_CHAR*)&optval,
                        sizeof (optval)) != 0)
        {
            return GT_ERROR;
        }
        if (setsockopt (sock, SOL_SOCKET, SO_RCVBUF, (GT_CHAR*)&optval,
                        sizeof (optval)) != 0)
        {
            return GT_ERROR;
        }
    }
    return sock;
}


/**
* @internal osSocketTcpDestroy function
* @endinternal
*
* @brief   Destroy a TCP socket
*
* @param[in] socketFd                 - Socket descriptor
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
GT_STATUS osSocketTcpDestroy 
(
    GT_SOCKET_FD   socketFd 
)
{
    struct linger ling;

    ling.l_onoff = 1;
    ling.l_linger = 5;

    /* attempt send any pending data before closing */
    setsockopt(socketFd, SOL_SOCKET, SO_LINGER,
               (GT_CHAR*) &ling, sizeof(struct linger));

    errno = 0; 
    if (close((int)socketFd)==0)
       return GT_OK;
    else
       return GT_ERROR;
}


/**
* @internal osSocketUdpDestroy function
* @endinternal
*
* @brief   Destroy a UDP socket
*
* @param[in] socketFd                 - Socket descriptor
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
GT_STATUS osSocketUdpDestroy 
(
    IN  GT_SOCKET_FD   socketFd 
)
{
    errno = 0 ;
    /* TBD - Do we need to call shutdown(sockeFd,2) ;*/
    if (close((int)socketFd)==0)
       return GT_OK ;
    else
       return GT_ERROR ;
}


/**
* @internal osSocketCreateAddr function
* @endinternal
*
* @brief   Build a TCP/IP address to be used across all other functions
*
* @param[in] ipAddr                   - String represents a dotted decimal IP address, such as
*                                      "10.2.40.10"
*                                      If ipAddr == NULL function will return INADDR_ANY
* @param[in] ipPort                   - IP Port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
GT_STATUS osSocketCreateAddr
(
    IN  const GT_CHAR *  ipAddr,
    IN  GT_SOCKET_PORT   ipPort,
    IN  GT_VOID **       sockAddr,
    IN  GT_SIZE_T *      sockAddrLen 
)
{
    errno = 0;
    if (sockAddr == NULL || sockAddrLen == NULL)
    {
        return GT_FAIL ;
    }

    if(socketAddrPool == 0)
    {
        if(gtPoolCreatePool(sizeof(struct sockaddr_in), 
                            GT_4_BYTE_ALIGNMENT,
                            SOCKET_ADDR_POOL_SIZE, 
                            GT_TRUE, &socketAddrPool) != GT_OK)
        {
            return GT_FAIL;
        }
    }

    *sockAddr = (struct sockaddr_in*) gtPoolGetBuf(socketAddrPool);
    
    if (*sockAddr == NULL)
    {
        return GT_FAIL;
    }
    
    osMemSet(*sockAddr, 0, sizeof(struct sockaddr_in));
    *sockAddrLen = sizeof(struct sockaddr_in) ;

    ((struct sockaddr_in*)(*sockAddr))->sin_family = AF_INET;
    ((struct sockaddr_in*)(*sockAddr))->sin_len = sizeof(struct sockaddr_in);
    ((struct sockaddr_in*)(*sockAddr))->sin_port = htons((int)ipPort) ;

    if (ipAddr == NULL)
    {
        ((struct sockaddr_in*)(*sockAddr))->sin_addr.s_addr = INADDR_ANY;
        return GT_OK;
    }
    if (inet_aton(ipAddr, &(((struct sockaddr_in*)(*sockAddr))->sin_addr)) == 0)
    {
        gtPoolFreeBuf(socketAddrPool,*sockAddr);
        return GT_FAIL ;
    }

    return GT_OK;
}


/**
* @internal osSocketDestroyAddr function
* @endinternal
*
* @brief   Destroy a TCP/IP address object built via osSocketBuildAddr()
*/
GT_VOID osSocketDestroyAddr
(
    IN  GT_VOID *       sockAddr
)
{
    if( socketAddrPool != 0 )
    {
        gtPoolFreeBuf(socketAddrPool,sockAddr);
        sockAddr = NULL;
    }
}


/**
* @internal osSocketBind function
* @endinternal
*
* @brief   Bind a name to a TCP or UDP socket
*
* @param[in] socketFd                 - Socket descriptor
* @param[in] sockAddr                 - A pointer to the local socket address structure,
*                                      probably filled out by osSocketBuildAddr.The caller
*                                      is unaware of what is the sockAddr type.
* @param[in] sockAddrLen              - Length of socketAddr
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
GT_STATUS osSocketBind
(
    IN  GT_SOCKET_FD     socketFd ,
    IN  GT_VOID *        sockAddr,
    IN  GT_SIZE_T        sockAddrLen
)
{
    errno = 0 ;

    if (bind ((int) socketFd, 
              (struct sockaddr*) sockAddr, 
              (socklen_t) sockAddrLen) == 0)
       return GT_OK ;
    else
    {
        osPrintf("osSocketBind:bind failed: %s\n", strerror(errno));
        return GT_FAIL;
    }
}


/**
* @internal osSocketListen function
* @endinternal
*
* @brief   Listen for new connections on a TCP socket
*
* @param[in] socketFd                 - Socket descriptor
* @param[in] maxConnections           - Number of connections to queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*
* @note Used only for TCP connection oriented sockets
*
*/
GT_STATUS osSocketListen
(
    IN  GT_SOCKET_FD     socketFd ,
    IN  GT_SSIZE_T       maxConnections
)
{
    errno = 0 ;
    if (listen( (int)socketFd, (int)maxConnections)== 0)
       return GT_OK ;
    else
       return GT_FAIL ;
}


/**
* @internal osSocketAccept function
* @endinternal
*
* @brief   Extracts the first TCP connection request on the queue of pending
*         connections and creates a new connected socket.
* @param[in] socketFd                 - Socket descriptor
* @param[in] sockAddr                 - A pointer to a socket address structure, represents the
*                                      client (target) address that requests the conenction.
*                                      If the server is not interested in the Client's protocol
*                                      information, a NULL should be provided.
* @param[in] sockAddrLen              - A value-result parameter, initialized to the size of
*                                      the allocated buffer sockAddr, and modified on return
*                                      to indicate the actual size of the address stored there.
*                                      If the server is not interested in the Client's protocol
*                                      information, a NULL should be provided.
*
* @retval socketFd                 - Socket descriptor
*
* @note Used only for TCP connection oriented sockets
*
*/
GT_SOCKET_FD osSocketAccept
(
    IN  GT_SOCKET_FD     socketFd ,
    IN  GT_VOID *        sockAddr,
    IN  GT_SIZE_T *      sockAddrLen
)
{
    GT_SOCKET_FD sock ;
    int optval = 1 ;
    socklen_t len = 0;
    
    if (sockAddrLen != NULL)
        len = (socklen_t)(*sockAddrLen);
    errno = 0 ;
    sock = accept ( (int) socketFd, 
                    (struct sockaddr*) sockAddr, 
                    sockAddrLen ? &len : NULL);

    if (sockAddrLen != NULL)
        *sockAddrLen = (GT_SIZE_T)len;

    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,  (GT_CHAR*) &optval, sizeof(optval));
    return sock ;
}


/**
* @internal osSocketConnect function
* @endinternal
*
* @brief   Connect socket to a remote address:
*         For UDP sockets, specifies the address to which datagrams are sent by
*         default, and the only address from which datagrams are received. For
*         TCP sockets, used by the TCP client that attempts to make a connection
*         to the TCP server.
* @param[in] socketFd                 - Socket descriptor
* @param[in] sockAddr                 - A pointer to a socket address structure, represents the
*                                      remote address to connect to. For TCP, it is the server
*                                      address.
* @param[in] sockAddrLen              - Length of sockAddr
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
GT_STATUS osSocketConnect
(
    IN  GT_SOCKET_FD     socketFd ,
    IN  GT_VOID *        sockAddr,
    IN  GT_SIZE_T        sockAddrLen
)
{    

    errno = 0;    

    if (connect ((int) socketFd, 
        (struct sockaddr*) sockAddr, 
        (socklen_t)sockAddrLen) != 0)
    {
        return GT_FAIL;
    }

    return GT_OK;
}


/**
* @internal osSocketSetNonBlock function
* @endinternal
*
* @brief   Set a socket option to be non-blocked
*
* @param[in] socketFd                 - Socket descriptor
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
GT_STATUS osSocketSetNonBlock
(
    IN  GT_SOCKET_FD     socketFd
)
{
    int flags;
 
    if ((flags = fcntl((int)socketFd, F_GETFL, 0))  < 0)
       return GT_FAIL; 

    flags |= O_NONBLOCK ;

    if (fcntl((int)socketFd, F_SETFL, flags)  < 0)
        return GT_FAIL ;
    
    return GT_OK ;
}
  

/**
* @internal osSocketSetBlock function
* @endinternal
*
* @brief   Set a socket option to be blocked
*
* @param[in] socketFd                 - Socket descriptor
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
GT_STATUS osSocketSetBlock
(
    IN  GT_SOCKET_FD     socketFd
)
{
    int flags ;
    
    if ((flags = fcntl((int)socketFd, F_GETFL, 0))  < 0)
       return GT_FAIL ; 

    flags &= ~O_NONBLOCK ;

    if (fcntl((int)socketFd, F_SETFL, flags)  < 0)
        return GT_FAIL ; 

    return GT_OK ;
}
  

/**
* @internal osSocketSend function
* @endinternal
*
* @brief   Send data to a TCP socket
*
* @param[in] socketFd                 - Socket descriptor
* @param[in] buff                     - Buffer to send
* @param[in] buffLen                  - Max length to send
*                                       Number of bytes sent
*/
GT_SSIZE_T osSocketSend
(
    IN  GT_SOCKET_FD   socketFd ,
    IN  const GT_VOID *buff,
    IN  GT_SIZE_T      buffLen
)
{
    return (GT_SSIZE_T) send((int)socketFd,        /* Socket FD                 */
                          buff,                 /* Buffer to send            */
                          (size_t)buffLen,      /* Max length to send        */
                          0);                   /* Flags                     */
}


/**
* @internal osSocketSendTo function
* @endinternal
*
* @brief   Send data to a UDP socket
*
* @param[in] socketFd                 - Socket descriptor
* @param[in] buff                     - Buffer to send
* @param[in] buffLen                  - Max length to send
* @param[in] sockAddr                 - A pointer to the targent socket address structure,
*                                      probably filled out by osSocketBuildAddr.  The caller
*                                      is unaware of what is the sockAddr type.
* @param[in] sockAddrLen              - Length of socketAddr
*                                       Number of bytes sent
*/
GT_SSIZE_T osSocketSendTo
(
    IN  GT_SOCKET_FD   socketFd ,
    IN  const GT_VOID *buff,
    IN  GT_SIZE_T      buffLen,
    IN  GT_VOID *      sockAddr,
    IN  GT_SIZE_T      sockAddrLen
)
{
    return (GT_SSIZE_T) sendto((int)socketFd,             /* Socket FD                */
                            buff,                      /* Buffer to receive        */
                            (size_t)buffLen,           /* Max length to received   */
                            0,                         /* Flags                    */
                            (struct sockaddr*)sockAddr,/* Socket address recv from */
                            (socklen_t)sockAddrLen);   /* Size of socket address   */ 
}

/**
* @internal osSocketRecv function
* @endinternal
*
* @brief   Receive data from a TCP socket
*
* @param[in] socketFd                 - Socket descriptor
* @param[in] buff                     - Buffer to receive
* @param[in] buffLen                  - Max length to send
* @param[in] removeFlag               - Flag indicate if to remove or no remove the data from
*                                      the socket buffer
*                                       Number of bytes sent
*/
GT_SSIZE_T osSocketRecv
(
    IN  GT_SOCKET_FD   socketFd,
    IN  GT_VOID *      buff,
    IN  GT_SIZE_T      buffLen,
    IN  GT_BOOL        removeFlag
)
{
    int flags = 0 ;
    if (removeFlag==GT_FALSE)
        flags = MSG_PEEK ;
  
    return (GT_SSIZE_T) recv((int)socketFd,        /* Socket FD                */
                          buff,                 /* Buffer to receive        */
                          (size_t)buffLen,      /* Max length to received   */
                          flags);               /* Flags                    */
}


/**
* @internal osSocketRecvFrom function
* @endinternal
*
* @brief   Receive data from a UDP socket
*
* @param[in] socketFd                 - Socket descriptor
* @param[in] buff                     - Buffer to send
* @param[in] buffLen                  - Max length to send
* @param[in] removeFlag               - Flag indicate if to remove or no remove the data from
*                                      the socket buffer
*
* @param[out] sockAddr                 - A pointer to a socket address structure, represents
*                                      the source (target) address that has sent the packet.
*                                      the caller can then compare the address later.
*                                      If this is NULL, this information will not be provided.
* @param[out] sockAddrLen              - A value-result parameter, initialized to the size of
*                                      the allocated buffer sockAddr, and modified on return
*                                      to indicate the actual size of the address stored there.
*                                       Number of bytes sent
*/
GT_SSIZE_T osSocketRecvFrom
(
    IN  GT_SOCKET_FD   socketFd ,
    IN  GT_VOID *      buff,
    IN  GT_SIZE_T      buffLen,
    IN  GT_BOOL        removeFlag,
    OUT GT_VOID *      sockAddr,
    OUT GT_SIZE_T *    sockAddrLen
)
{
    int flags = 0 ;
    socklen_t fromLen = 0;
    GT_SSIZE_T ret;

    if (sockAddrLen != NULL)
        fromLen = (socklen_t)(*sockAddrLen);

    if (removeFlag==GT_FALSE)
        flags = MSG_PEEK ;
  
    ret = (GT_SSIZE_T) recvfrom((int)socketFd,           /* Socket FD                */
                              buff,                      /* Buffer to receive        */
                              (size_t)buffLen,           /* Max length to received   */
                              flags,                     /* Flags                    */
                              (struct sockaddr*)sockAddr,/* Socket address recv from */
                              sockAddrLen ? & fromLen : NULL);/* Size of socket address */ 

    if (sockAddrLen != NULL)
        *sockAddrLen = (GT_SIZE_T)fromLen;

    return ret;
}


/**
* @internal osSocketSetSocketNoLinger function
* @endinternal
*
* @brief   Set the socket option to be no linger when closing connection.
*
* @param[in] socketFd                 - Socket descriptor
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
GT_STATUS osSocketSetSocketNoLinger
(
    IN  GT_SOCKET_FD     socketFd
)
{
    SOCKET_LINGER sockLinger;
    
    sockLinger.lOnOff = 1; /* disable the linger option */
    sockLinger.lLinger = 0;

    if (setsockopt (socketFd, SOL_SOCKET, SO_LINGER, (GT_CHAR*)&sockLinger,
                    sizeof (sockLinger)) != 0)
        return GT_FAIL;
    
    return GT_OK ;
}

/**
* @internal osSocketExtractIpAddrFromSocketAddr function
* @endinternal
*
* @brief   Extracts the IP address from sockaddr_in structure.
*
* @param[in] sockAddr                 - pointer to sockaddr_in.
* @param[in] ipAddr                   - pointer to address in GT_U32 format to be filled by the
*                                      routine.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
GT_STATUS osSocketExtractIpAddrFromSocketAddr
(
    IN  GT_VOID*   sockAddr,
    OUT GT_U32*    ipAddr
)
{
    /* extract the ip address from the socket address struct */
    *ipAddr = ntohl(((struct sockaddr_in*)(sockAddr))->sin_addr.s_addr);

    return GT_OK ;
}

/**
* @internal osSocketGetSocketAddrSize function
* @endinternal
*
* @brief   Returns the size of sockaddr_in.
*
* @param[out] sockAddrSize             - to be filled with sockaddr_in size.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*
* @note USERS THAT IMPLEMENTS THEIR OWN OS LAYER CAN RETURN SIZE = 0.
*
*/
GT_STATUS osSocketGetSocketAddrSize
(
    OUT GT_U32*    sockAddrSize
)
{
    *sockAddrSize = sizeof(struct sockaddr_in);

    return GT_OK ;
}

/**
* @internal osSocketShutDown function
* @endinternal
*
* @brief   The call allows either direction of the TCP connection to be closed,
*         independent of the other direction.
*         Even though shutdown will eventually terminate the TCP connection,
*         it does not close the socket or frees its resources.
*         In order to close the socket and free its resources only close can be used.
* @param[in] socketFd                 - File descriptor
*                                      SOCKET_SHUTDOWN_OPERATION -
*                                      SOCKET_SHUTDOWN_CLOSE_INPUT,
*                                      SOCKET_SHUTDOWN_CLOSE_OUTPUT,
*                                      SOCKET_SHUTDOWN_CLOSE_ALL
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
GT_STATUS osSocketShutDown 
(
  IN GT_SOCKET_FD               socketFd ,
  IN SOCKET_SHUTDOWN_OPERATION     operation
)
{
    if (shutdown (socketFd, operation) != 0)
        return GT_ERROR;

    return GT_OK;
}



