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
* @file main.c
*
* @brief Initialization functions for the simulation on Unix-like OS
*
*
* @version   25
********************************************************************************
*/
#include <ctype.h>
#define __USE_BSD
#define __USE_XOPEN2K
#define __USE_MISC
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef CMD_LUA_CLI
#include <termios.h>
#endif
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <gtOs/gtOsGen.h>
#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsSem.h>
#include <gtOs/gtOsInit.h>
#include <gtOs/gtOsSharedMemory.h>
#include <gtStack/gtOsSocket.h>
#define CMDOS_NO_CMDCOMMON
#include <cmdShell/os/cmdOs.h>
#include <cpss/generic/version/cpssGenStream.h>
#include <asicSimulation/SInit/sinit.h>
#include <os/simTypesBind.h>
#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/version/cpssDxChVersion.h>
#endif
#ifdef PX_FAMILY
#include <cpss/px/version/cpssPxVersion.h>
#endif
#define EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
#include <os/simOsBindOwn.h>

/* commandLine should belong to simulation (simulation can be used by another client withot this var) */
extern char commandLine[1280];
char **cmdArgv;
char **cmdEnv;

/* IntLockmutex should belong to simulation (simulation can be used without Enabler)! */
extern GT_MUTEX IntLockmutex;
/* For Determining whether the app should run in Hot Insertion Removal Mode or Not */
extern GT_BOOL isHirApp;

extern int osSprintf(char * buffer, const char* format, ...);

extern const char *VERSION_DATE;
extern const char *VERSION_FLAGS;

#define BIND_FUNC(funcName)        \
                        simOsBindInfo.funcName = funcName

#define BIND_LEVEL_FUNC(level,funcName) \
            simOsBindInfo.level.funcName = funcName

SIM_OS_FUNC_BIND_STC simOsBindInfo;

static void simulationBindOsFuncs(void)
{
        /* reset all fields of simOsBindInfo */
        osMemSet(&simOsBindInfo,0,sizeof(simOsBindInfo));

        /* let the simulation OS bind it's own functions to the simulation */
        simOsFuncBindOwnSimOs();

        BIND_FUNC(osWrapperOpen);

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

        return;
}

void simulationStart (void)
{
    extern int userAppInit(void);

    char   semName[50];

    /* Must be called before use any of OS functions. */
    osWrapperOpen(NULL);
    /* create semaphores */

    osSprintf(semName, "IntLockmutex");

    /* create 'reentrant' mutex ! */
    if (osMutexCreate(semName, &IntLockmutex) != 0)
        exit (0);

    simulationBindOsFuncs();

#ifndef RTOS_ON_SIM
    /* call simulation init */
    simulationLibInit();
#endif
    /* init CMD */
    userAppInit();
}

static void Usage(const char *argv0)
{
    printf("Usage: %s [options]\n", argv0);
    printf("Options:\n");
    printf("    -help          - This help\n");
    printf("    -i _inifile_   - Use this .ini file for simulation\n");
    printf("    -e _inifile_   - Use built-in .ini file for simulation\n");
    printf("    -tty           - Use current tty for console\n");
    printf("    -hir           - Run in Hot Insert Removal Mode\n");
    printf("                     Enabled by default\n");
#ifdef CMD_LUA_CLI
    printf("    -cmdshell      - Force cmdshell (Galtis shell) at startup\n");
    printf("    -luacli        - Force luaCLI at startup\n");
    printf("    -config _file_ - Execute luaCLI script\n");
    printf("    -noconfig      - Ignore luaCLI startup script\n");
#endif
    printf("    -daemon        - daemonize, redirect stdout\n");
    printf("                     to /tmp/appDemo_stdout\n");
    printf("    -redir_stdout _filename_\n");
    printf("                   - Redirect stdout to file\n");
    printf("                     To be used with -daemon\n");
    exit(0);
}

static void appDemoPrintVersion()
{
    printf("CPSS Version Stream: %s\n",CPSS_STREAM_NAME_CNS);
#ifdef CHX_FAMILY
    printf("CPSS DXCH Version: %s\n",CPSS_DXCH_VERSION_CNS);
#endif
#ifdef PX_FAMILY
    printf("CPSS PX Version: %s\n",CPSS_PX_VERSION_CNS);
#endif
    printf("Build date: %s\n",VERSION_DATE);
    printf("Compilation flags: %s\n",VERSION_FLAGS);
    exit(0);
}

#ifdef CMD_LUA_CLI
static struct termios tc_saved;
static void restore_tty(void)
{
    tcsetattr(0, TCSAFLUSH, &tc_saved);
    printf("tty settings restored\n");
}
#endif
extern GT_BOOL cmdStreamGrabSystemOutput;
int main(int argc, char * argv[], char *envp[])
{
    int i, len=0;
    const char *redir_stdout = NULL;
    
    isHirApp=GT_FALSE;
    for (i=1; i<argc; i++)
    {
        if ((strcmp(argv[i], "--version") == 0 ))
        {
           appDemoPrintVersion();
        }
        if (   (strcmp(argv[i], "-help") == 0)
            || (strcmp(argv[i], "--help") == 0)
            || (strcmp(argv[i], "-h") == 0)
            || (strcmp(argv[i], "-?") == 0)
            || (strcmp(argv[i], "?") == 0)
            || (strcmp(argv[i], "/h") == 0)
            || (strcmp(argv[i], "/?") == 0))
        {
            Usage(argv[0]);
        }
        if (strcmp(argv[i], "-daemon") == 0)
        {
            if (daemon(1, 1) < 0)
                perror("Failed to daemonize");
            setenv("PSEUDO", "notty", 1);
            redir_stdout = "/tmp/appDemo_stdout";
        }
        if ((i+1 < argc) && (strcmp(argv[i],"-redir_stdout") == 0))
        {
            redir_stdout = argv[++i];
        }
    }
    if (redir_stdout != NULL) /* Only when daemon or forced */
    {
        i = open(redir_stdout, O_WRONLY|O_CREAT|O_APPEND, 0644);
        if (i < 0)
        {
            perror("open(redir_stdout_file) failed");
        }
        else
        {
            if (i != 1)
            {
                close(1);
                dup2(i, 1);
            }
            if (i != 2)
            {
                close(2);
                dup2(i, 2);
            }
        }
    }

#ifdef SHARED_MEMORY
    if(shrMemInitSharedLibrary_FirstClient() != GT_OK)
        return -1;
#endif

    cmdStreamGrabSystemOutput = GT_FALSE;
#ifdef CMD_LUA_CLI
    cmdDefaultShellType = CMD_STARTUP_SHELL_LUACLI_E;
    /* set cmdOsConfigFilePath to current directory */
    {
        char tmppath[4096];
        if (getcwd(tmppath, sizeof(tmppath)) == 0)
        {
            fprintf(stderr, "getcwd() : %s\n", strerror(errno));
        }
        strcat(tmppath, "/");
        cmdOsConfigFilePath = malloc(strlen(tmppath)+1);
        strcpy(cmdOsConfigFilePath, tmppath);
    }
#endif /* CMD_LUA_CLI */

    cmdArgv = calloc(argc+1, sizeof(char*));
    if (cmdArgv == NULL)
    {
        fprintf(stderr, "calloc(%d, %d) failed: %s\n",
                argc+1, sizeof(char*), strerror(errno));
        return 1;
    }
    cmdEnv = envp;
    commandLine[0] = 0;
    for (i=0; i<argc; i++)
    {
        len += strlen(argv[i]);
        cmdArgv[i] = argv[i];

        if ((unsigned)len < sizeof(commandLine)-2) /* leave place for ' ' and '\0' */
        {
            strcat(commandLine,argv[i]);

            if (i<argc-1)
            {
                strcat(commandLine," ");
                len++;
            }
        }

        /* no need to define environment variable PSEUDO to cause simulation
         * print output on local console
         */
        /* new key "-tty" causes all input/output be on local console */
#if 0
        /* just ignore this option, always act as if it set */
        if (i > 0 && !strcmp(argv[i], "-tty"))
        {
        }
#endif

        /* new key equale to '-stdout comPort' */
        if (i > 0 && !strcmp(argv[i], "-grabstd"))
        {
            cmdStreamGrabSystemOutput = GT_TRUE;
        }

        /* to enable hot insert remove feature  */
        if (i > 0 && !strcmp(argv[i], "-hir"))
        {
            isHirApp = GT_TRUE;
        }

        /* support "-stdout" key like in window's simulation */
        if (i > 0 && (i+1 < argc) && !strcmp(argv[i], "-stdout"))
        {
            if (strcmp(argv[i+1], "comPort") == 0 ||
                    strcmp(argv[i+1], "comport") == 0)
            {
                cmdStreamGrabSystemOutput = GT_TRUE;
            }
        }

#ifdef CMD_LUA_CLI
        /* support "-cmdshell" key to set cmdShell starttup shell */
        if (strcmp("-cmdshell", argv[i]) == 0) {
            cmdDefaultShellType = CMD_STARTUP_SHELL_CMDSHELL_E;
            continue;
        }
        if (strcmp("-luacli", argv[i]) == 0) {
            cmdDefaultShellType = CMD_STARTUP_SHELL_LUACLI_E;
            continue;
        }
        /* support "-config" key to enter the name of the configuration file */
        if (strcmp("-config", argv[i]) == 0) {
            i++;
            cmdOsConfigFileName = malloc(strlen(argv[i])+1);
            strcpy(cmdOsConfigFileName, argv[i]);
            continue;
        }
        if (strcmp("-noconfig", argv[i]) == 0)
        {
            cmdOsConfigFileName = NULL;
        }
#endif /* CMD_LUA_CLI */

    }
    /* all input/output be on local console if PSEUDO is not set yet */
    if (getenv("PSEUDO") == NULL)
        setenv("PSEUDO", "/dev/tty", 1);

    cmdArgv[argc] = 0;
    /* extend cmdArgv[0] to full path if it is relative */
    if (cmdArgv[0][0] != '/')
    {
        char argv0[4096];
        if (getcwd(argv0, sizeof(argv0)) == 0)
        {
            fprintf(stderr, "getcwd() : %s\n", strerror(errno));
        }
        strcat(argv0, "/");
        strcat(argv0, cmdArgv[0]);
        cmdArgv[0] = malloc(strlen(argv0)+1);
        strcpy(cmdArgv[0], argv0);
    }

#ifdef CMD_LUA_CLI
    tcgetattr(0, &tc_saved);
    atexit(restore_tty);
#endif
    if (osStartEngine(argc, (const char**)argv, "appDemoSim", simulationStart) != GT_OK)
        return 1;

    return 0;
}



