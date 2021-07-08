/*****************************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <winsock.h>

#include <gtOs/gtGenTypes.h>
#include <gtOs/gtOsGen.h>
#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsTimer.h>
#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsStr.h>
#include <gtStack/gtOsSocket.h>
#define CMDOS_NO_CMDCOMMON
#include <cmdShell/os/cmdOs.h>
#include <direct.h>

#ifdef ASIC_SIMULATION
    #include <asicSimulation/SInit/sinit.h>
    #include <os/simTypesBind.h>
    #define EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
    #include <os/simOsBindOwn.h>
    #undef EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES

    #define BIND_FUNC(funcName) \
            simOsBindInfo.funcName = funcName

    #define BIND_LEVEL_FUNC(level,funcName)     \
            simOsBindInfo.level.funcName = funcName
#endif


/* For Determining whether the app should run in Hot Insertion Removal Mode or Not */
extern GT_BOOL isHirApp;

#define DEBUG_FILENAME "batch.cmd"

GT_STATUS userAppInitialize(
    IN GT_U32 devNum
);

#ifdef CMD_DB_DEBUG

/* for stoping on error in WINDOWS environment command file debug */
GT_BOOL fileTest = GT_FALSE;



/**
* @internal cmdInitDebug function
* @endinternal
*
* @brief   initialization and debug method for windows implementation.
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - failed to allocate memory for socket
* @retval GT_FAIL                  - on error
*/
GT_STATUS cmdInitDebug(GT_8 *cmdfile)
{
    GT_U32 taskTCPIP = 0, taskSerial = 0, taskSwitchCmd = 0;
    GT_STREAM *serialStream;
    GT_STATUS retVal;

    /* stop when error occors in command file testing */
    fileTest = GT_TRUE;

    /*hir feature is disabled by default */
    isHirApp=GT_FALSE;


    isLinkedListInitialized = GT_FALSE;
    /*cmdLlHead = NULL;*/

    #ifndef ASIC_SIMULATION
        cmdSwitchingServerPort = CMD_SWITCHING_BASE_SOCKET_PORT;
    #endif /*ASIC_SIMULATION*/

    if (osWrapperOpen(NULL) != GT_OK)
    {   /* os wrapper failed to initialize */
        osPrintf("osWrapper initialization failure!\n");
        return GT_FAIL;
    }

    if (osSocketInit() != GT_OK)
    {   /* os wrapper failed to initialize */
        osPrintf("osWrapperEx initialization failure!\n");
        return GT_FAIL;
    }

    /* was before - osMutexCreate("parser", &parserMutex) */
    if (cmdParseInit(NULL) == GT_FAIL ||
        osMutexCreate("server", &serverMutex) != GT_OK)
    {
        osPrintf("failed to create mutex descriptors!\n");
        return GT_FAIL;
    }

    /* check if we already running */
    if (globalInit++)
        return GT_FAIL;

    /* initialize commander database */
    if (cmdCpssInitDatabase() != GT_OK)
    {
        osPrintf("failed to initialize command database!\n");
        return GT_FAIL;
    }

    /* add test app level commands to commander database */
    if (cmdUserInitDatabase() != GT_OK)
    {
        osPrintf("failed to initialize command database!\n");
        return GT_FAIL;
    }

    if ((retVal = osTaskCreate (
                    "CMD_Switching",
                    CMD_THREAD_PRIO,
                    32768,
                    (unsigned (__TASKCONV *)(void*)) createMultiTapiServerTask,
                    NULL,
                    &taskSwitchCmd)) != GT_OK)
    {
        return retVal;
    }

    /* initialize serial server thread */
    if (cmdStreamCreate(
            STREAM_FILE | STREAM_FILTER_CRLF,
            (GT_U32)cmdfile,
            &serialStream) != GT_OK)
    {
        osPrintf("commander: failed to create serial stream\n");
    }
    else
        if (osTaskCreate(
                "SERIAL",
                CMD_THREAD_PRIO,        /* thread priority      */
                32768,                  /* thread stack size    */
                (unsigned (__TASKCONV *)(void*)) cmdEventLoopThread,
                serialStream,           /* thread arguments     */
                &taskSerial) != GT_OK)
        {
            osPrintf("commander: failed to spawn serial thread\n");
            taskSerial = 0;
        }

    osPrintf("commander: threads are running %s\n\n", cmdfile);
    return GT_OK;
}
#endif

#ifdef ASIC_SIMULATION
/**
* @internal simulationInit function
* @endinternal
*
* @brief   initialize the Simulation section .
*/
void simulationInit
(
    void
)
{
    SIM_OS_FUNC_BIND_STC simOsBindInfo;
    static int initDone = 0;

    if(initDone)
    {
        /* allow to call this before cpssInitSystem */
        return;
    }

    /* Must be called before use any of OS functions. */
    osWrapperOpen(NULL);

    /**********************************************/
    /* bind the simulation with extra OS services */
    /**********************************************/
    {
        /* reset all fields of simOsBindInfo */
        osMemSet(&simOsBindInfo,0,sizeof(simOsBindInfo));

        /* let the simulation OS bind it's own functions to the simulation */
        simOsFuncBindOwnSimOs();

        BIND_FUNC(osWrapperOpen);
        /*BIND_FUNC(osSerialDestroySerial);*/

        BIND_LEVEL_FUNC(sockets,osSocketTcpCreate);
        BIND_LEVEL_FUNC(sockets,osSocketUdpCreate);
        BIND_LEVEL_FUNC(sockets,osSocketTcpDestroy);
        BIND_LEVEL_FUNC(sockets,osSocketUdpDestroy);
        BIND_LEVEL_FUNC(sockets,osSocketCreateAddr);
        BIND_LEVEL_FUNC(sockets,osSocketDestroyAddr);
        BIND_LEVEL_FUNC(sockets,osSocketBind);
        BIND_LEVEL_FUNC(sockets,osSocketListen);
        BIND_LEVEL_FUNC(sockets,osSocketAccept);
        BIND_LEVEL_FUNC(sockets,osSocketConnect);
        BIND_LEVEL_FUNC(sockets,osSocketSetNonBlock);
        BIND_LEVEL_FUNC(sockets,osSocketSetBlock);
        BIND_LEVEL_FUNC(sockets,osSocketSend);
        BIND_LEVEL_FUNC(sockets,osSocketSendTo);
        BIND_LEVEL_FUNC(sockets,osSocketRecv);
        BIND_LEVEL_FUNC(sockets,osSocketRecvFrom);
        BIND_LEVEL_FUNC(sockets,osSocketSetSocketNoLinger);
        BIND_LEVEL_FUNC(sockets,osSocketExtractIpAddrFromSocketAddr);
        BIND_LEVEL_FUNC(sockets,osSocketGetSocketAddrSize);
        BIND_LEVEL_FUNC(sockets,osSocketShutDown);

        BIND_LEVEL_FUNC(sockets,osSelectCreateSet);
        BIND_LEVEL_FUNC(sockets,osSelectEraseSet);
        BIND_LEVEL_FUNC(sockets,osSelectZeroSet);
        BIND_LEVEL_FUNC(sockets,osSelectAddFdToSet);
        BIND_LEVEL_FUNC(sockets,osSelectClearFdFromSet);
        BIND_LEVEL_FUNC(sockets,osSelectIsFdSet);
        BIND_LEVEL_FUNC(sockets,osSelectCopySet);
        BIND_LEVEL_FUNC(sockets,osSelect);
        BIND_LEVEL_FUNC(sockets,osSocketGetSocketFdSetSize);

        /* this needed for binding the OS of simulation with our OS functions */
        simOsFuncBind(&simOsBindInfo);

    }

    /************************************************/
    /* init the kernel of simulation -- the PPs,FAs */
    /************************************************/
    simulationLibInit();

    /* state that init was done */
    initDone = 1;

    return;
}
#endif /*ASIC_SIMULATION*/

extern GT_BOOL cmdStreamGrabSystemOutput;
/**
* @internal main function
* @endinternal
*
* @brief   enrty point for WIN32 application.
*
* @retval 0                        - allways
*
* @note command line: [-serial <com_number> | -batch <file> |
*       -unit <unit_number>] ...
*       the defaults: -serial 0, -unit 0
*       for WM simulation [-stdout <stdOut>]
*       stdOut is one of:
*       1. comPort
*       2. console (default)
*
*/
int main(GT_32 argc, GT_8 *argv[])
{
    int i, rsNo = 0, unitNum = 0;
    char* file = NULL ;

    {
        GT_STATUS simOsAllocConsole(void);
        /* bind the console and allow the close of console to trigger
           'Clean exit functions' - see implementation of simOsAllocConsole()
        */
        simOsAllocConsole();
    }
    cmdStreamGrabSystemOutput = GT_FALSE;

#ifdef CMD_LUA_CLI
    cmdDefaultShellType = CMD_STARTUP_SHELL_LUACLI_E;
    /* set cmdOsConfigFilePath to current directory */
    {
        char tmppath[4096];
        _getcwd(tmppath, sizeof(tmppath));
        strcat(tmppath, "\\");
        cmdOsConfigFilePath = _strdup(tmppath);
    }
#endif /* CMD_LUA_CLI */

    for (i = 1; i < argc; i++)
    {
        /* new key equale to '-stdout comPort' */
        if (strcmp("-grabstd", argv[i]) == 0)
        {
            cmdStreamGrabSystemOutput = GT_TRUE;
        }
        /* the next arguments are parametrized */
        if (i + 1 >= argc)
            break;
   

        if (strcmp("-batch", argv[i]) == 0)
        {
            file = argv[++i];
            continue;
        }

        /* to enable hot insert remove feature  */
        if (i > 0 && !strcmp(argv[i], "-hir"))
        {
            isHirApp = GT_TRUE;
        }



        if (strcmp("-serial", argv[i]) == 0)
        {
            cmdDefaultShellType = CMD_STARTUP_SHELL_CMDSHELL_E;
            rsNo = atoi(argv[++i]);
            continue;
        }

        if (strcmp("-unit", argv[i]) == 0)
        {
            unitNum = atoi(argv[++i]) ;
            continue;
        }

        if (strcmp("-stdout", argv[i]) == 0)
        {
            i++;
            /* check where to send the STD output */
            if(strcmp("comPort",argv[i]) == 0 ||
               strcmp("comport",argv[i]) == 0)
            {
                /* send this LAST printing to the console */
                osPrintf(" stdOut is now the com port \n");
                cmdStreamGrabSystemOutput = GT_TRUE;
            }
            continue;
        }

#ifdef CMD_LUA_CLI
        /* support "-cmdshell" key to set cmdShell starttup shell */
        if (strcmp("-cmdshell", argv[i]) == 0) {
            cmdDefaultShellType = CMD_STARTUP_SHELL_CMDSHELL_E;
            continue;
        }
        /* support "-config" key to enter the name of the configuration file */
        if (strcmp("-config", argv[i]) == 0) {
            i++;
            cmdOsConfigFileName = _strdup(argv[i]);
            continue;
        }
#endif /* CMD_LUA_CLI */

    }

    /* Port for messages exchanging sockets */
    /*appDemoMsgExchangingServerPort = MSG_EXCHANGING_BASE_SOCKET_PORT +
                                    ((unitNum > 0) ? unitNum : 0);*/
#if 0
    /* Port for Multi-TAPI sockets */
    cmdSwitchingServerPort = CMD_SWITCHING_BASE_SOCKET_PORT +
                                    ((unitNum > 0) ? unitNum : 0);
#endif
   /* cmdInit operns COM<rsNo + 1> */
   if (rsNo > 0)
       rsNo -- ;

/* init the simulation only after the command line is activated ,
   to allow terminal to do commands if the init of the simulation stuck */
    simulationInit();

    userAppInitialize(rsNo);

    while (1)
        osTimerWkAfter(10000);

    /*Unreachable code*/
    return 0;
}



