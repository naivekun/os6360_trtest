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
* @file cmdMain.c
*
* @brief command shell entry point implementation
*
*
* @version   31
********************************************************************************
*/

/***** Include files ***************************************************/

#include <cmdShell/common/cmdCommon.h>
#include <cmdShell/common/cmdExtServices.h>

#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/shell/cmdMain.h>
#include <cmdShell/shell/cmdEngine.h>
#include <cmdShell/shell/cmdConsole.h>
#include <cmdShell/shell/cmdSystem.h>
#include <cmdShell/shell/cmdParser.h>
#include <cmdShell/shell/userInitCmdDb.h>
#include <cmdShell/os/cmdStreamImpl.h>
#include <cmdShell/FS/cmdFS.h>


#define MAX_NUM_UNITS 128
#define MSG_EXCHANGING_BASE_SOCKET_PORT 6000


#ifdef LINUX_SIM
    #include <string.h>
#endif

/* definition for Multi-TAPI unit switching */
#define CMD_SWITCHING_BASE_SOCKET_PORT  8000

/***** Variables *******************************************************/
static GT_32 globalInit = 0;

extern IOStreamPTR defaultStdio;

/* ports for Galtis command switching, must by initialized */
/* the value may be overridden                             */
GT_U16 cmdSwitchingServerPort = CMD_SWITCHING_BASE_SOCKET_PORT;

/* Unit to IP Mapping linked list */
/*STRUCT_LL_HEAD *cmdLlHead;*/

/* cpssInitSystem has been run */
extern GT_BOOL systemInitialized;

/* Unit is connected/not connected to Galtis */
#if (defined PSS_PRODUCT) || (defined CPSS_PRODUCT)
extern
#else
static
#endif
GT_BOOL terminalUnit;

/* for task thread safe */
GT_SEM parserMutex = 0;
GT_SEM serverMutex = 0;

GT_U32 taskSerial = 0;

/* Stdio sem -  only one stream can have the stdio */
GT_MUTEX  gStdioMtx;

/***** Public Functions ************************************************/

GT_STATUS tcpKeepAliveTask(
        IOStreamPTR stream
)
{
    char buf[100];
    int k;
    while (stream->connected(stream))
    {
        if ((k = stream->read(stream, buf, sizeof(buf))) > 0)
            stream->write(stream, buf, k);
    }
    stream->destroy(stream);
    return GT_OK;
}

/* extern */
GT_STATUS cmdEventLoop
(
    IOStreamPTR IOStream
);

GT_STATUS tcpServerInstance(
        IOStreamPTR stream
)
{
    /* start keepalive here */
    cmdEventLoop(stream);
    stream->destroy(stream);
    return GT_OK;
}

#ifdef WNC_CPSS_IPC
/* extern */
GT_STATUS wncIpcToCpssStart(void);
#endif /* WNC_CPSS_IPC */

#ifdef CMD_LUA_CLI
IOStreamPTR IOStreamCreateStdout(void);
GT_STATUS luaCLI_EventLoop
(
    IOStreamPTR IOStream
);
GT_STATUS luaCLI_tcpInstance(
        IOStreamPTR stream
)
{
    /* add telnet protocol layer */
    IOStreamPTR telnetStream;
    telnetStream = IOStreamCreateTelnet(stream);
    if (telnetStream == NULL)
    {
        stream->destroy(stream);
        return GT_FAIL;
    }
    /* It is a place to add
     * login/password check here over telnetStream
     */

    luaCLI_EventLoop(telnetStream);
    telnetStream->destroy(telnetStream);
    return GT_OK;
}
GT_STATUS luaCLI_LoadConfigurationFile(
  IN  IOStreamPTR IOStream,
  OUT GT_VOID_PTR *luaInstancePtrPtr
);

/* WNC_LUA_CLI_ENHANCE_IOP start */
static CPSS_OS_SIG_SEM         semId_WNC_IOP = 0;
static char                    *semName_WNC_IOP = "semId_WNC_IOP";

GT_STATUS luaCLI_EventLoop_WNC_IOP
(
    IOStreamPTR IOStream, void * luaState
);

void * luaCLI_Engine_PreAllocate
(
    IOStreamPTR IOStream
);
typedef struct lua_PreAllocat_s {
    IOStreamPTR telnetStream;
    void *L;
}lua_PreAllocat_t ;

static lua_PreAllocat_t luaResource[1];

#define SEND_CMD3(a1,a2,a3) { \
cmdBuf[0] = a1; \
cmdBuf[1] = a2; \
cmdBuf[2] = a3; \
stream->transport->write(stream->transport, cmdBuf, 3); \
}
#define TELNET_IAC  255
#define TELNET_WILL 251
#define TELNET_OPT_ECHO 1
#define TELNET_OPT_SGA  3

typedef enum {
    OPT_DISABLED,
    OPT_ENABLED,
    OPT_DISABLED_WAIT_ACK,
    OPT_ENABLED_WAIT_ACK
} TELNET_OPTION_STATUS_T;

typedef struct {
    IOStreamSTC common;
    IOStreamPTR transport;
    enum {
        RSTATE_NONE,
        RSTATE_IAC,
        RSTATE_IACDONT,
        RSTATE_IACDO,
        RSTATE_IACWONT,
        RSTATE_IACWILL
    } readState;
    TELNET_OPTION_STATUS_T opt_TRANSMIT_BINARY;
    TELNET_OPTION_STATUS_T opt_ECHO;
    TELNET_OPTION_STATUS_T opt_SGA;

} telnetStreamSTC, *telnetStreamPTR;

GT_STATUS luaCLI_tcpInstance_WNC_IOP(
    IOStreamPTR socket
)
{
    int i;
    telnetStreamPTR stream;
    IOStreamPTR telnetStream;
    unsigned char cmdBuf[3];

    do {
        /* looking for pre-allocated resource */
        for (i = 0; i<(sizeof(luaResource)/sizeof(lua_PreAllocat_t)); i++)
        {
            if (stream = (telnetStreamPTR)luaResource[i].telnetStream)
            {
                cmdOsSigSemWait(semId_WNC_IOP, CPSS_OS_SEM_WAIT_FOREVER_CNS);
                if (!(stream->common.flags & IO_STREAM_FLAG_USED))
                {
                    /* found an unused resource, make it is used. */
                    stream->common.flags |= IO_STREAM_FLAG_USED;
                    cmdOsSigSemSignal(semId_WNC_IOP);

                    /* replace socket */
                    stream->transport = socket;

                    /* set telnet protocol options */
                    stream->readState = RSTATE_NONE;
                    stream->opt_TRANSMIT_BINARY = OPT_ENABLED;

                    SEND_CMD3(TELNET_IAC, TELNET_WILL, TELNET_OPT_ECHO);
                    stream->opt_ECHO = OPT_ENABLED_WAIT_ACK;

                    SEND_CMD3(TELNET_IAC, TELNET_WILL, TELNET_OPT_SGA);
                    stream->opt_SGA = OPT_ENABLED_WAIT_ACK;

                    luaCLI_EventLoop_WNC_IOP(stream, luaResource[i].L);

                    cmdOsSigSemWait(semId_WNC_IOP, CPSS_OS_SEM_WAIT_FOREVER_CNS);
                    stream->common.destroy(stream);
                    cmdOsSigSemSignal(semId_WNC_IOP);
                    return GT_OK;
                }
                cmdOsSigSemSignal(semId_WNC_IOP);
            }
        }
    } while (1);

    return GT_OK;
}

GT_STATUS __TASKCONV lua_PreAllocateTask(GT_VOID *params)
{
    int i, status;

    status = cpssOsSigSemBinCreate(semName_WNC_IOP, CPSS_OS_SEMB_FULL_E, &semId_WNC_IOP);
    if (status != GT_OK)
    {
        cmdOsPrintf("lua_PreAllocateTask: failed to create semaphore\n");
        return GT_FAIL;
    }

    for (i = 0; i<(sizeof(luaResource)/sizeof(lua_PreAllocat_t)); i++)
    {
        if ((luaResource[i].telnetStream = IOStreamCreateTelnet_PreAllocate()) == NULL)
        {
            cmdOsPrintf("commander: failed to allocate telnetStream at [%d]\n", i);
            return GT_FAIL;
        }
        else
        {
            if ((luaResource[i].L = (void*)luaCLI_Engine_PreAllocate(luaResource[i].telnetStream)) == NULL)
            {
                cmdOsPrintf("commander: failed to allocate lua_State at [%d]\n", i);
                return GT_FAIL;
            }
        }
    }

    cmdStreamSocketServiceCreate(
        "luaCLI_WNC_IOP",
        NULL, 54321/*port*/,
        luaCLI_tcpInstance_WNC_IOP,
        GT_FALSE/*multipleInstances*/,
        GT_TRUE/*isConsole*/);
    /* initialize LUA CLI */
    if (cmdStreamSocketServiceStart("luaCLI_WNC_IOP") != GT_OK)
    {
        cmdOsPrintf("commander: failed to create WNC IOP listening task\n");
    }

    cmdOsPrintf("Pre allocate lua CLI resource successfully.\n");

    return GT_OK;
}
/* WNC_LUA_CLI_ENHANCE_IOP end */

#endif /* CMD_LUA_CLI */
#ifdef  CMD_FS
GT_STATUS cmdFileTransferServiceStart(void);
GT_STATUS cmdTFTPDstart(void);
#endif
#ifdef _WIN32
IOStreamPTR cmdStreamCreateWin32Console(void);
#endif

#ifdef CMD_INCLUDE_TCPIP
static const char* tcpServices[] = {
    "luaCLI",
    "cmdShell",
    NULL
};
#endif

/**
* @internal cmdInit function
* @endinternal
*
* @brief   commander initialization (entry point); this function spawns
*         configured streaming threads and immediately returns to caller.
* @param[in] devNum                   - The device number of the serial port.
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - failed to allocate memory for socket
* @retval GT_FAIL                  - on error
*
* @note For a new pemanent connection (like serial one) add the following:
*       IOStreamPTR stream;
*       ...
*       stream = cmdStreamCreate....();
*       cmdStreamStartEventLoopHandler(...);
*       To accept tcp connections on second port:
*       cmdStreamSocketServiceCreate(...);
*       cmdStreamSocketServiceStart(...);
*
*/
GT_STATUS cmdInit
(
    IN  GT_U32  devNum
)
{
#ifdef CMD_INCLUDE_SERIAL
    GT_TASK taskSerial = 0;
#endif

#ifdef  CMD_LUA_CLI
/* WNC_LUA_CLI_ENHANCE_IOP start */
    GT_U32  lua_PreAllocateTaskId;
/* WNC_LUA_CLI_ENHANCE_IOP end */
#endif /* CMD_LUA_CLI */

    GT_STATUS retVal;

#ifndef PSS_PRODUCT
    /* initialize external services (can't call directly to mainOs functions)*/
    if(GT_OK != cmdInitExtServices())
    {
        return GT_FAIL;
    }
#endif

    #ifndef ASIC_SIMULATION
        cmdSwitchingServerPort = CMD_SWITCHING_BASE_SOCKET_PORT;
    #endif /*ASIC_SIMULATION*/

#if (defined PSS_PRODUCT) || (defined CPSS_PRODUCT)
    if (osWrapperOpen(NULL) != GT_OK)
    {   /* os wrapper failed to initialize */
        cmdOsPrintf("osWrapper initialization failure!\n");
        return GT_FAIL;
    }
#endif

    if (cmdStreamSocketInit() != GT_OK)
    {   /* os wrapper failed to initialize */
        cmdOsPrintf("cmdStreamSocketInit() initialization failure!\n");
        return GT_FAIL;
    }
#ifdef CMD_INCLUDE_SERIAL
    if (cmdStreamSerialInit() != GT_OK)
    {   /* os wrapper failed to initialize */
        cmdOsPrintf("cmdStreamSocketInit() initialization failure!\n");
        return GT_FAIL;
    }
#endif

    /* was before - osMutexCreate("parser", &parserMutex) */
    if (cmdParseInit(NULL) == GT_FAIL ||
        cmdOsMutexCreate("server", &serverMutex) != GT_OK)
    {
        cmdOsPrintf("failed to create mutex descriptors!\n");
        return GT_FAIL;
    }

    /* check if we already running */
    if (globalInit++)
        return GT_FAIL;
#ifdef IMPL_GALTIS
    /* initialize commander database */
    if (cmdCpssInitDatabase() != GT_OK)
    {
        cmdOsPrintf("failed to initialize command database!\n");
        return GT_FAIL;
    }
#endif
    /* add test app level commands to commander database */
    if (cmdUserInitDatabase() != GT_OK)
    {
        cmdOsPrintf("failed to initialize command database!\n");
        return GT_FAIL;
    }
    if (cmdEngineInit() != GT_OK)
    {
            cmdOsPrintf("failed to initialize command engine!\n");
       return GT_FAIL;
    }
    terminalUnit = GT_TRUE;
    cmdStreamSocketServiceCreate(
        "CMD_Switching",
        "127.0.0.1", cmdSwitchingServerPort,
        tcpServerInstance,
        GT_FALSE/*multipleInstances*/,
        GT_FALSE/*cmdOsConsole*/);
    if ((retVal = cmdStreamSocketServiceStart("CMD_Switching")) == GT_OK)
    {
        /* do nothing */
    }
    else
    {
        /* If connection is requested, then commands are not executed directly
           on this unit */
        terminalUnit = GT_FALSE;
    }

    /* Allocate stdio semaphore */
    if (cmdOsMutexCreate("cmdStdioMtx", &gStdioMtx) != GT_OK)
    {
        cmdOsPrintf("failed to initialize stdio semaphore!\n");
        return GT_FAIL;
    }


#ifdef CMD_INCLUDE_SERIAL
    defaultStdio = cmdStreamCreateSerial(devNum);
    if (defaultStdio != NULL)
    {
        cmdTakeGlobalStdio(defaultStdio);
        cmdReleaseGlobalStdio();
    }
#else
    defaultStdio = cmdStreamCreateNULL();
#endif /* CMD_INCLUDE_SERIAL */
#ifdef  _WIN32
    if (devNum == 0)
    {
        /* if appDemo started with -serial 0 use console */
        if (defaultStdio != NULL)
        {
            defaultStdio->destroy(defaultStdio);
        }
        defaultStdio = cmdStreamCreateWin32Console();
        cmdTakeGlobalStdio(defaultStdio);
        cmdReleaseGlobalStdio();
    }
#endif

    if (defaultStdio == NULL)
    {
#ifdef CMD_LUA_CLI
        defaultStdio = IOStreamCreateStdout();
#else
        defaultStdio = IOStreamCreateNULL();
#endif
    }
    cmdStreamRedirectStdout(defaultStdio);

#if 0
#ifdef _WIN32
    {
        extern GT_U32 simulationInitReady;

        while(simulationInitReady == 0)
        {
            /* wait for simulation to be in operational state */
            cmdOsTimerWkAfter(500);
        }
    }
#endif /*_WIN32*/
#endif


#ifdef CMD_INCLUDE_TCPIP
#ifdef  CMD_LUA_CLI

    cmdStreamSocketServiceCreate(
            "luaCLI",
            NULL, 12345/*port*/,
            luaCLI_tcpInstance,
            GT_TRUE/*multipleInstances*/,
            GT_TRUE/*isConsole*/);
    /* initialize LUA CLI */
    if (cmdStreamSocketServiceStart("luaCLI") != GT_OK)
    {
        cmdOsPrintf("commander: failed to create LUA CLI listening task\n");
    }
#endif
#ifdef  CMD_FS
    cmdFSinit();
    cmdFileTransferServiceStart();
    cmdTFTPDstart();

#endif
    cmdStreamSocketServiceCreate(
            "cmdShell",
            NULL, CMD_SERVER_PORT,
            tcpServerInstance,
            GT_TRUE/*multipleInstances*/,
            GT_TRUE/*isConsole*/);
    if (cmdStreamSocketServiceStart("cmdShell") != GT_OK)
    {
        cmdOsPrintf("commander: failed to create tcpip server thread\n");
    }
    cmdStreamSocketServiceCreate(
            "TCPKEEPALIVE",
            NULL, CMD_KA_SERVER_PORT,
            tcpKeepAliveTask,
            GT_TRUE/*multipleInstances*/,
            GT_FALSE/*isConsole*/);
    if (cmdStreamSocketServiceStart("TCPKEEPALIVE") != GT_OK)
    {
        cmdOsPrintf("commander: failed to create tcpip keepalive task thread\n");
    }
#endif /* CMD_INCLUDE_TCPIP */

    {
        char services[80] = {0};
#ifdef CMD_INCLUDE_TCPIP
        int i;
        for (i = 0; tcpServices[i]; i++)
        {
            if (cmdStreamSocketServiceIsRunning(tcpServices[i]) == GT_TRUE)
            {
                cmdOsStrCat(services, " ");
                cmdOsStrCat(services, tcpServices[i]);
            }
        }
#endif
        if (taskSerial)
        {
            cmdOsStrCat(services, " SERIAL");
        }
        if (!services[0])
        {
            cmdOsStrCpy(services, " NONE");
        }
        cmdOsPrintf("commander: threads are running:%s\n\n", services);
    }

#if 0
    /* initialize remote ip detection thread */
    if (cmdRemoteIpDetectionServerCreate() != GT_OK)
    {
        cmdOsPrintf("commander: fail to create remoteip detection server thread\n");
    }
#endif

#ifdef CMD_LUA_CLI
    luaCLI_LoadConfigurationFile(
#ifdef CMD_INCLUDE_SERIAL
            defaultStdio
#else
            NULL
#endif
            , &(defaultStdio->customPtr));

#endif

#ifdef CMD_INCLUDE_SERIAL
    defaultStdio->flags |= IO_STREAM_FLAG_C_FIRSTSHELL;
    /* initialize serial server thread */
    if (cmdStreamStartEventLoopHandler(
                "SERIAL",
                cmdEventLoop,
                defaultStdio,
                GT_TRUE/*isConsole*/,
                &taskSerial) != GT_OK)
    {
        cmdOsPrintf("commander: failed to spawn serial thread\n");
        taskSerial = 0;
    }
#endif /* CMD_INCLUDE_SERIAL */

#ifdef CMD_INCLUDE_TCPIP
#ifdef  CMD_LUA_CLI
/* WNC_LUA_CLI_ENHANCE_IOP start */
/*
 * Create a thread to pre-allocate lua CLI resource
 */
    if (cmdOsTaskCreate(
                "lua_PreAlloc",
                CMD_THREAD_PRIO,     /* thread priority      */
                0x8000,              /* thread stack size    */
                lua_PreAllocateTask,
                NULL,
                &lua_PreAllocateTaskId) != GT_OK)
    {
        cmdOsPrintf("commander: failed to allocate LUA CLI resource task\n");
    }

/* WNC_LUA_CLI_ENHANCE_IOP end */
#endif /* CMD_LUA_CLI */
#endif /* CMD_INCLUDE_TCPIP */

    #ifdef WNC_CPSS_IPC
        wncIpcToCpssStart();
    #endif /* WNC_CPSS_IPC */

    return GT_OK;
}


