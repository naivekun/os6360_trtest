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
* @file cmdStreamSocket.c
*
* @brief This file contains common functions for mainCmd socket streams
*
* @version   6
********************************************************************************
*/
#include <cmdShell/os/cmdStreamImpl.h>
#include <extUtils/IOStream/IOStreamTCP.h>

/*********** generic socket stream implementation *****************************/


/*********** Socket listener implementation **********************************/

/**
* @internal osSocketTcpCreateListen function
* @endinternal
*
* @brief   Create TCP socket, bind it and start listening for incoming connections
*
* @param[in] ip                       -  address to listen on
*                                      NULL means listen on all interfaces
* @param[in] port                     - tcp  for incoming connections
*
* @param[out] socketPtr                - Store socket descriptior
*                                       GT_OK
*                                       GT_FAIL
*/
GT_STATUS osSocketTcpCreateListen
(
    IN  char *ip,
    IN  int port,
    OUT GT_SOCKET_FD *socketPtr
)
{
    GT_SOCKET_FD    socket;
    GT_VOID*        bindAddr;
    GT_SIZE_T       bindAddrLen;

    if (!socketPtr)
        return GT_BAD_PARAM;

    /* create master server socket and start listening on specified ip/port */
    socket = cmdOsSocketTcpCreate(GT_SOCKET_DEFAULT_SIZE);
    if (socket < 0)
        return GT_FAIL;

    if (cmdOsSocketCreateAddr(ip, port, &bindAddr, &bindAddrLen) != GT_OK)
    {
        cmdOsSocketTcpDestroy(socket);
        return GT_FAIL;
    }

    if (cmdOsSocketBind(socket, bindAddr, bindAddrLen) != GT_OK)
    {
        cmdOsSocketDestroyAddr(bindAddr);
        cmdOsSocketTcpDestroy(socket);
        return GT_FAIL;
    }

    cmdOsSocketDestroyAddr(bindAddr);
    if (cmdOsSocketListen(socket, 32) != GT_OK)
    {
        cmdOsSocketTcpDestroy(socket);
        return GT_FAIL;
    }

    *socketPtr = socket;
    return GT_OK;
}

#if 0
/*
 * Typedef: struct LISTENER_PARAMS_STC
 *
 * Description:
 *      This structure is used to pass listener parameters
 *      to newly created listener task
 *
 * Fields:
 *      masterSocket
 *      streamTaskName
 *      handler
 *      multipleInstances
 *      isConsole
 */
typedef struct LISTENER_PARAMS_STC {
    GT_SOCKET_FD masterSocket;
    char *streamTaskName;
    eventLoopHandlerPTR handler;
    GT_BOOL multipleInstances;
    GT_BOOL isConsole;
} LISTENER_PARAMS_STC;

/*******************************************************************************
* cmdStreamSocketListenerTask
*
* DESCRIPTION:
*       Socket listener task
*
* INPUTS:
*       params
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*       None
*
*******************************************************************************/
static GT_VOID __TASKCONV cmdStreamSocketListenerTask
(
    LISTENER_PARAMS_STC* params
)
{
    GT_SOCKET_FD socket;
    IOStreamPTR  stream;
    int instance = 0;

    while (1)
    {
        socket = cmdOsSocketAccept(params->masterSocket, NULL, NULL);
        if (socket < 0)
        {
            /* handle? */
            continue;
        }
        if ((stream = cmdStreamCreateSocket(socket)) == NULL)
        {
            cmdOsSocketTcpDestroy(socket);
            /* handle? */
            continue;
        }
        if (params->multipleInstances == GT_FALSE)
        {
            stream->isConsole = params->isConsole;
            params->handler(stream);
        }
        else
        {
            char taskname[80];
            GT_TASK taskId;

            cmdOsSprintf(taskname, "%s%d", params->streamTaskName, instance++);

            if (cmdStreamStartEventLoopHandler(
                        taskname,
                        params->handler,
                        stream,
                        params->isConsole,
                        &taskId) != GT_OK)
            {
                stream->destroy(stream);
            }
        }
    }
}

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
)
{
    GT_SOCKET_FD    masterSocket;
    LISTENER_PARAMS_STC *params;

    if (osSocketTcpCreateListen(ip, port, &masterSocket) != GT_OK)
    {
        return GT_FAIL;
    }

    /* Now create listener task */
    params = (LISTENER_PARAMS_STC *)cmdOsMalloc(sizeof(*params));
    if (!params)
    {
        cmdOsSocketTcpDestroy(masterSocket);
        return GT_FAIL;
    }
    params->masterSocket = masterSocket;
    params->streamTaskName = streamTaskName;
    params->handler = handler;
    params->multipleInstances = multipleInstances;
    params->isConsole = isConsole;

    if (cmdOsTaskCreate(
                listenerTaskName,
                STREAM_THREAD_PRIO+1,   /* thread priority          */
                32768,                  /* use default stack size   */
                (unsigned (__TASKCONV *)(void*))
                    cmdStreamSocketListenerTask,
                params,
                taskId) != GT_OK)
    {
        cmdOsSocketTcpDestroy(masterSocket);
        cmdOsFree(params);
        return GT_FAIL;
    }

    return GT_OK;
}
#endif

/*
 * Typedef: struct SERVICE_PARAMS_STC
 *
 * Description:
 *      This structure is used as service description
 *      to newly created listener task
 *
 * Fields:
 *      serviceName
 *      masterSocket
 *      streamTaskName
 *      handler
 *      multipleInstances
 *      isConsole
 */
typedef struct SERVICE_PARAMS_STC {
    const char *serviceName;
    char *ip;
    int port;
    eventLoopHandlerPTR handler;
    GT_BOOL multipleInstances;
    GT_BOOL isConsole;
    GT_SOCKET_FD masterSocket;
    GT_BOOL running;
    int instance;
    GT_TASK taskId;
} SERVICE_PARAMS_STC;
#define SERVICES_MAX 10

static SERVICE_PARAMS_STC services[SERVICES_MAX];
static int numServices = 0;

#ifdef CMD_LUA_CLI
GT_STATUS luaGlobalGetIntVal(
  IN  const char *key,
  OUT GT_U32     *valuePtr
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
)
{
    int i;
    SERVICE_PARAMS_STC *s;
    for (i = 0; i < numServices; i++)
    {
        if (cmdOsStrCmp(serviceName, services[i].serviceName) == 0)
        {
            return GT_ALREADY_EXIST;
        }
    }
    if (numServices == SERVICES_MAX)
    {
        return GT_NO_RESOURCE;
    }
    s = &(services[numServices]);
    cmdOsMemSet(s, 0, sizeof(*s));

    s->serviceName = serviceName;
    if (ip)
    {
        s->ip = (char*)cmdOsMalloc(cmdOsStrlen(ip)+1);
        cmdOsStrCpy(s->ip, ip);
    }
    s->port = port;
#ifdef CMD_LUA_CLI
    {
        /* override port with _p_${serviceName} from luaGlobal, if exists */
        char varname[80];
        GT_U32 portNum;

        cmdOsSprintf(varname, "_p_%s", s->serviceName);
        if (luaGlobalGetIntVal(varname, &portNum) == GT_OK)
        {
            s->port = (int)portNum;
        }
    }
#endif
    s->handler = handler;
    s->multipleInstances = multipleInstances;
    s->isConsole = isConsole;
    s->running = GT_FALSE;

    numServices++;
    return GT_OK;
}

/*******************************************************************************
* cmdStreamSocketServiceListenerTask
*
* DESCRIPTION:
*       Socket service listener task
*
* INPUTS:
*       params
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*       None
*
*******************************************************************************/
static GT_VOID __TASKCONV cmdStreamSocketServiceListenerTask
(
    SERVICE_PARAMS_STC* params
)
{
    GT_SOCKET_FD socket;
    IOStreamPTR stream;
    GT_VOID*     rdSet;
    rdSet = cmdOsSelectCreateSet();
    while (params->running == GT_TRUE)
    {
        /* Use select to check if service need to be stopped */
        cmdOsSelectZeroSet(rdSet);
        cmdOsSelectAddFdToSet(rdSet, params->masterSocket);

        if (cmdOsSelect(params->masterSocket+1, rdSet, NULL, NULL, 1000) <= 0)
            continue;

        socket = cmdOsSocketAccept(params->masterSocket, NULL, NULL);
        if (socket < 0)
        {
            /* handle? */
            continue;
        }
        if ((stream = IOStreamCreateSocket(socket)) == NULL)
        {
            cmdOsSocketTcpDestroy(socket);
            /* handle? */
            continue;
        }
        if (params->multipleInstances == GT_FALSE)
        {
            stream->isConsole = params->isConsole;
            params->handler(stream);
        }
        else
        {
            char taskname[80];
            GT_TASK taskId;

            cmdOsSprintf(taskname, "%s_%d", params->serviceName, params->instance++);

            if (cmdStreamStartEventLoopHandler(
                        taskname,
                        params->handler,
                        stream,
                        params->isConsole,
                        &taskId) != GT_OK)
            {
                stream->destroy(stream);
            }
        }
    }
    cmdOsSelectEraseSet(rdSet);
}


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
)
{
    char listenerTaskName[80];
#define SERVICE_FIND() \
    int i; \
    SERVICE_PARAMS_STC *s; \
    for (i = 0; i < numServices; i++) \
    { \
        if (cmdOsStrCmp(serviceName, services[i].serviceName) == 0) \
        { \
            break; \
        } \
    } \
    if (i == numServices) \
    { \
        return GT_NO_SUCH; \
    } \
    s = &(services[i]);

    SERVICE_FIND();
    if (s->running == GT_TRUE)
    {
        return GT_OK;
    }

    if (osSocketTcpCreateListen(s->ip, s->port, &(s->masterSocket)) != GT_OK)
    {
        return GT_FAIL;
    }
    
    cmdOsSprintf(listenerTaskName, "%s_lsn", s->serviceName);
    s->running = GT_TRUE;
    /* Now create listener task */
    if (cmdOsTaskCreate(
                listenerTaskName,
                STREAM_THREAD_PRIO+1,   /* thread priority          */
                32768,                  /* use default stack size   */
                (unsigned (__TASKCONV *)(void*))
                    cmdStreamSocketServiceListenerTask,
                s,
                &(s->taskId)) != GT_OK)
    {
        cmdOsSocketTcpDestroy(s->masterSocket);
        s->running = GT_FALSE;
        return GT_FAIL;
    }
    return GT_OK;
}

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
)
{
    SERVICE_FIND();
    if (s->running == GT_FALSE)
    {
        return GT_OK;
    }

    s->running = GT_FALSE;
    cmdOsSocketTcpDestroy(s->masterSocket); /* listen() may be interrupted */
    s->masterSocket = -1;
    return GT_OK;
}

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
)
{
    int i;
    for (i = 0; i < numServices; i++)
    {
        if (cmdOsStrCmp(serviceName, services[i].serviceName) == 0)
        {
            return services[i].running;
        }
    }
    return GT_FALSE;
}

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
)
{
    SERVICE_FIND();

    if (portNumberPtr == NULL)
        return GT_BAD_PARAM;
    *portNumberPtr = (GT_U16)(s->port);
    return GT_OK;
}

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
)
{
    SERVICE_FIND();
    s->port = portNumber;
    return GT_OK;
}


