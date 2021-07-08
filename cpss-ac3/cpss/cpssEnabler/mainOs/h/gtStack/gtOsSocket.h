/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

#ifndef __gtOsSocket_h
#define __gtOsSocket_h

#ifdef __cplusplus
extern "C" {
#endif

#include <gtOs/gtGenTypes.h>
#include <gtStack/gtStackTypes.h>
#include <gtStack/gtOsSelect.h>


#define GT_SOCKET_DEFAULT_SIZE    -1

/* Socket file descriptor and socket port generic type */
#ifndef _WIN32
typedef int GT_SOCKET_FD ;
#else
typedef GT_UINTPTR GT_SOCKET_FD ;
#endif
typedef int GT_SOCKET_PORT ;


/*******************************************************************************
 * Typedef:     struct   SOCKET_LINGER
 *
 * Description: Structure sent to setsockopt in order to set the SO_LINGER
 *              option for the socket
 *
 * Fields :
 *       lOnOff     - enable/disable the linger option when closing socket.
 *       lLinger    - indicates the amount of time to linger.
 * Comments:
 *
 *******************************************************************************/
typedef struct
{
    GT_32   lOnOff;
    GT_32   lLinger;
}SOCKET_LINGER;



/**
* @enum SOCKET_SHUTDOWN_OPERATION
*/
typedef enum{

    SOCKET_SHUTDOWN_CLOSE_INPUT,

    SOCKET_SHUTDOWN_CLOSE_OUTPUT,

    SOCKET_SHUTDOWN_CLOSE_ALL

} SOCKET_SHUTDOWN_OPERATION;



/* Define socket's errors */
#define GT_SOCKET_ERROR_BASE    0x30000
/* Sockets definitions of regular Berkeley error constants */
#define GT_SOCKET_EAGAIN        (GT_SOCKET_ERROR_BASE + 11)
#define GT_SOCKET_EINTR         (GT_SOCKET_ERROR_BASE + 4)
#define GT_SOCKET_EINVAL        (GT_SOCKET_ERROR_BASE + 22)
#define GT_SOCKET_EMSGSIZE            (GT_SOCKET_ERROR_BASE + 36)
#define GT_SOCKET_EISCONN       (GT_SOCKET_ERROR_BASE + 56)
#define GT_SOCKET_SHUTDOWN      (GT_SOCKET_ERROR_BASE + 58)
#define GT_SOCKET_EWOULDBLOCK   (GT_SOCKET_ERROR_BASE + 70)
#define GT_SOCKET_EINPROGRESS   (GT_SOCKET_ERROR_BASE + 68)
#define GT_SOCKET_EALREADY      (GT_SOCKET_ERROR_BASE + 69)
#define GT_SOCKET_ECONNRESET    (GT_SOCKET_ERROR_BASE + 103)
#define GT_SOCKET_ECONNREFUSED  (GT_SOCKET_ERROR_BASE + 111)
#define GT_SOCKET_EBADF         (GT_SOCKET_ERROR_BASE + 9)
#define GT_SOCKET_ENETUNREACH   (GT_SOCKET_ERROR_BASE + 101)
#define GT_SOCKET_EHOSTUNREACH  (GT_SOCKET_ERROR_BASE + 113)

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
);

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
const char * osSocketLastErrorStr(void);

/**
* @internal osSocketTcpCreate function
* @endinternal
*
* @brief   Create a socket from TCP type
*/
GT_SOCKET_FD osSocketTcpCreate (GT_32 sockSize);


/**
* @internal osSocketUdpCreate function
* @endinternal
*
* @brief   sockSize - the size of the receive & send socket buffer. -1 indicates
*         the default size.
*/
GT_SOCKET_FD osSocketUdpCreate (GT_32 sockSize);


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
);


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
);


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
);


/**
* @internal osSocketDestroyAddr function
* @endinternal
*
* @brief   Destroy a TCP/IP address object built via osSocketBuildAddr()
*/
GT_VOID osSocketDestroyAddr
(
    IN  GT_VOID *       sockAddr
);


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
);


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
);


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
    IN  GT_SOCKET_FD     socketFd,
    IN  GT_VOID *        sockAddr,
    IN  GT_SIZE_T *      sockAddrLen
);


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
    IN  GT_SOCKET_FD     socketFd,
    IN  GT_VOID *        sockAddr,
    IN  GT_SIZE_T        sockAddrLen
);


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
);


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
);


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
);


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
);


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
);


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
);


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
);

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
);

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
);



/**
* @internal osSocketShutDown function
* @endinternal
*
* @brief   This routine shuts down all, or part, of a connection-based socket.
*
* @param[in] socketFd                 - Socket descriptor
* @param[in] operation                - 0 = receives disallowed
*                                      1 = sends disallowed
*                                      2 = sends and disallowed
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
GT_STATUS osSocketShutDown
(
  IN	GT_SOCKET_FD   				socketFd ,
  IN	SOCKET_SHUTDOWN_OPERATION 	operation
);


#ifdef __cplusplus
}
#endif

#endif  /* ifndef __gtOsSocket_h */
/* Do Not Add Anything Below This Line */



