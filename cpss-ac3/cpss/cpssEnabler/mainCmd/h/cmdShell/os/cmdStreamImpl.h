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
* @file cmdStreamImpl.h
*
* @brief This file contains types and routines for mainCmd I/O streams
*
* @version   2
********************************************************************************
*/
#ifndef __cmdStreamImplh
#define __cmdStreamImplh

#include <cmdShell/common/cmdCommon.h>
#include <extUtils/IOStream/IOStream.h>
#include <extUtils/IOStream/IOStreamTCP.h>

#define STREAM_THREAD_PRIO 5

/*******************************************************************************
* cmdStreamGrabSystemOutput
*
* DESCRIPTION:
*       cmdStreamRedirectStdout() will grab stdout and stderr streams
*       if this flags set to GT_TRUE
*
*******************************************************************************/
extern GT_BOOL cmdStreamGrabSystemOutput;

/**
* @internal cmdStreamRedirectStdout function
* @endinternal
*
* @brief   Redirect stdout/stderr to given stream
*
* @param[in] stream                   - destination stream
*                                       GT_OK
*                                       GT_FAIL
*/
GT_STATUS cmdStreamRedirectStdout
(
    IN IOStreamPTR stream
);

#if 0

/**
* @internal cmdStreamCreateNULL function
* @endinternal
*
* @brief   Create null stream. This stream equals to /dev/null
*
* @retval stream                   - null stream
*/
IOStreamPTR cmdStreamCreateNULL(void);
#endif

/**
* @internal cmdStreamSerialInit function
* @endinternal
*
* @brief   Initialize serial engine
*/
GT_STATUS cmdStreamSerialInit(void);

/**
* @internal cmdStreamSerialFinish function
* @endinternal
*
* @brief   Close serial engine
*/
GT_STATUS cmdStreamSerialFinish(void);

/**
* @internal cmdStreamCreateSerial function
* @endinternal
*
* @brief   Create serial port stream
*
* @param[in] devNum                   - the serial device port number (0 = COM1, ...)
*
* @retval stream                   - serial stream
*                                       NULL if error
*/
IOStreamPTR cmdStreamCreateSerial
(
    IN GT_U32 devNum
);

/**
* @internal cmdStreamSocketInit function
* @endinternal
*
* @brief   Initialize TCP/IP socket engine
*/
GT_STATUS cmdStreamSocketInit(void);

/**
* @internal cmdStreamSocketFinish function
* @endinternal
*
* @brief   Close socket engine
*/
GT_STATUS cmdStreamSocketFinish(void);

#if 0
/**
* @internal cmdStreamCreateSocket function
* @endinternal
*
* @brief   Create socket stream
*
* @param[in] socket                   -  descriptor
*
* @retval stream                   - socket stream
*                                       NULL if error
*/
IOStreamPTR cmdStreamCreateSocket
(
    IN GT_SOCKET_FD socket
);

/**
* @internal cmdStreamCreateTelnet function
* @endinternal
*
* @brief   Create Telnet protocol stream
*
* @param[in] socket                   -  I/O stream
*
* @retval stream                   - socket stream
*                                       NULL if error
*/
IOStreamPTR cmdStreamCreateTelnet
(
    IN IOStreamPTR socket
);

#endif

#if defined(_linux) || defined(__FreeBSD__)
/**
* @internal cmdStreamCreatePipe function
* @endinternal
*
* @brief   Create pipe stream
*
* @retval stream                   - pipe stream
*                                       NULL if error
*
* @note Input and output file descriptors must be specified in environment
*       variables PIPE_STREAM_INFD and PIPE_STREAM_OUTFD
*
*/
IOStreamPTR cmdStreamCreatePipe(void);
#endif /* defined(_linux) || defined(__FreeBSD__) */


/*
 * Typedef: eventLoopHandlerPTR
 *
 * Description:
 *      Pointer to application stream handler
 */
typedef GT_STATUS (*eventLoopHandlerPTR)(
        IOStreamPTR stream
);

/**
* @internal cmdStreamStartEventLoopHandler function
* @endinternal
*
* @brief   Create new task to run stream handler
*
* @param[in] taskName                 - task name for handler
* @param[in] handler                  - pointer to  function
* @param[in] stream                   - stream
* @param[in] isConsole                - application flag
*
* @param[out] taskId                   - pointer for task ID
*                                       GT_OK
*                                       GT_FAIL
*/
GT_STATUS cmdStreamStartEventLoopHandler
(
    IN  char *taskName,
    IN  eventLoopHandlerPTR handler,
    IN  IOStreamPTR stream,
    IN  GT_BOOL isConsole,
    OUT GT_TASK *taskId
);

#if 0
/**
* @internal cmdStreamSocketCreateListenerTask function
* @endinternal
*
* @brief   Create socket listenet task
*         This task will accept incoming socket connections and then start
*         handler for each connection
* @param[in] listenerTaskName         - name for listener task
* @param[in] ip                       -  address to listen on
*                                      NULL means listen on all interfaces
* @param[in] port                     tcp  for incoming connections
* @param[in] streamTaskName           - task name for handler
* @param[in] handler                  - pointer to  function
* @param[in] multipleInstances        - Allow more than one handler at time
* @param[in] isConsole                - application flag
*
* @param[out] taskId                   - pointer for listener task ID
*                                       GT_OK
*                                       GT_FAIL
*/
GT_STATUS cmdStreamSocketCreateListenerTask(
    IN  char *listenerTaskName,
    IN  char *ip,
    IN  int port,
    IN  char *streamTaskName,
    IN  eventLoopHandlerPTR handler,
    IN  GT_BOOL multipleInstances,
    IN  GT_BOOL isConsole,
    OUT GT_TASK *taskId
);
#endif

/**
* @internal cmdStreamSocketServiceCreate function
* @endinternal
*
* @brief   Create socket listener service
*
* @param[in] serviceName              - service name
* @param[in] ip                       -  address to listen on
*                                      NULL means listen on all interfaces
* @param[in] port                     tcp  for incoming connections
* @param[in] handler                  - pointer to  function
* @param[in] multipleInstances        - Allow more than one handler at time
* @param[in] isConsole                - application flag
*                                       GT_OK
*                                       GT_FAIL
*/
GT_STATUS cmdStreamSocketServiceCreate(
    IN  const char *serviceName,
    IN  char *ip,
    IN  GT_U16 port,
    IN  eventLoopHandlerPTR handler,
    IN  GT_BOOL multipleInstances,
    IN  GT_BOOL isConsole
);

/**
* @internal cmdStreamSocketServiceStart function
* @endinternal
*
* @brief   Start socket service
*
* @param[in] serviceName
*/
GT_STATUS cmdStreamSocketServiceStart(
    IN  const char *serviceName
);

/**
* @internal cmdStreamSocketServiceStop function
* @endinternal
*
* @brief   Stop socket service
*
* @param[in] serviceName
*/
GT_STATUS cmdStreamSocketServiceStop(
    IN  const char *serviceName
);

/**
* @internal cmdStreamSocketServiceIsRunning function
* @endinternal
*
* @brief   Return service running status
*
* @param[in] serviceName
*/
GT_BOOL cmdStreamSocketServiceIsRunning(
    IN  const char *serviceName
);

/**
* @internal cmdStreamSocketServiceGetPort function
* @endinternal
*
* @brief   Get service port number
*
* @param[in] serviceName
*
* @param[out] portNumberPtr
*/
GT_STATUS cmdStreamSocketServiceGetPort(
    IN  const char *serviceName,
    OUT GT_U16     *portNumberPtr
);

/**
* @internal cmdStreamSocketServiceSetPort function
* @endinternal
*
* @brief   Get service port number
*
* @param[in] serviceName
* @param[in] portNumber
*/
GT_STATUS cmdStreamSocketServiceSetPort(
    IN  const char *serviceName,
    IN  GT_U16     portNumber
);


#endif /* __cmdStreamImplh */


