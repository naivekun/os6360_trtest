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
* @brief linux entry point implementation
*
*
* @version   8
********************************************************************************
*/
#define _BSD_SOURCE
#define __NO_STRING_INLINES
#define _DEFAULT_SOURCE
#include <gtOs/gtOsSharedMemory.h>
#include <gtOs/gtOsInit.h>
#define CMDOS_NO_CMDCOMMON
#include <cmdShell/os/cmdOs.h>
#ifdef CMD_LUA_CLI
#include <termios.h>
#endif
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cpss/generic/version/cpssGenStream.h>
#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/version/cpssDxChVersion.h>
#endif
#ifdef PX_FAMILY
#include <cpss/px/version/cpssPxVersion.h>
#endif

#ifdef SHARED_MEMORY

/*#include <gtOs/gtOsTimer.h>*/
GT_BOOL isSlaveProcess = GT_FALSE;


extern GT_STATUS cmdInit(IN GT_U32 devNum);

void cmdAppInit(void)
{
    GT_STATUS rc;
    rc = cmdInit(0);
    if (rc != GT_OK)
    {
        fprintf(stderr, "cmdInit() failed, rc=%d\n", rc);
    }
}

#endif

extern const char *VERSION_DATE;
extern const char *VERSION_FLAGS;
extern int userAppInit(void);

void preUserAppInit(void)
{
    /* here we will map the pss memory */
    
    userAppInit();
}

static void Usage(const char *argv0)
{
    printf("Usage: %s [options]\n", argv0);
    printf("Options:\n");
    printf("    -help          - This help\n");
    printf("    -tty           - Use current tty for console\n");
    printf("                     Enabled by default\n");
#ifdef CMD_LUA_CLI
    printf("    -cmdshell      - Force cmdshell (Galtis shell) at startup\n");
    printf("    -luacli        - Force luaCLI at startup\n");
    printf("    -config _file_ - Execute luaCLI script\n");
    printf("    -noconfig      - Ignore luaCLI startup script\n");
    printf("    -hir           - Run in Hot Insert Removal Mode\n");    
#endif
#ifdef SHARED_MEMORY
    printf("    -slave         - run a process from the secondary terminal\n");
    printf("                     that behaves as appDemo.\n");
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


extern GT_BOOL cmdStreamGrabSystemOutput;
extern GT_BOOL isHirApp;
#ifdef CMD_LUA_CLI
static struct termios tc_saved;
static void restore_tty(void)
{
    tcsetattr(0, TCSAFLUSH, &tc_saved);
    printf("tty settings restored\n");
}
#endif


int main(int argc, const char * argv[])
{
    int i, fd;
#ifdef CMD_LUA_CLI
    GT_BOOL cmdOsConfigFileNameAlloc = GT_FALSE;
#endif
    const char *redir_stdout = NULL;
    /*hir feature is disabled by default */
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
        cmdStreamGrabSystemOutput = GT_FALSE;
        fd = open(redir_stdout, O_WRONLY|O_CREAT|O_APPEND, 0644);
        if (fd < 0)
        {
            perror("open(redir_stdout_file) failed");
        }
        else
        {
            if (fd != 1)
            {
                close(1);
                dup2(fd, 1);
            }
            if (i != 2)
            {
                close(2);
                dup2(fd, 2);
            }
        }
    }
/*
#ifdef SHARED_MEMORY
    if(shrMemInitSharedLibrary_FirstClient() != GT_OK)
        return -1;
#endif
*/

#ifdef CMD_LUA_CLI
    cmdDefaultShellType = CMD_STARTUP_SHELL_LUACLI_E;
#endif

    for (i=1; i < argc; i++)
    {
#ifdef CMD_LUA_CLI
        if (strcmp("-cmdshell", argv[i]) == 0)
        {
            cmdDefaultShellType = CMD_STARTUP_SHELL_CMDSHELL_E;
        }
        if (strcmp("-luacli", argv[i]) == 0)
        {
            cmdDefaultShellType = CMD_STARTUP_SHELL_LUACLI_E;
        }
        if (strcmp("-hir", argv[i]) == 0)
        {
            isHirApp=GT_TRUE;
        }

        /* support "-config" key to enter the name of the configuration file */
        if ((strcmp("-config", argv[i]) == 0) && (i + 1 < argc))
        {
            i++;
            if( GT_TRUE == cmdOsConfigFileNameAlloc )
            {
                free(cmdOsConfigFileName);
            }
            cmdOsConfigFileName = malloc(strlen(argv[i])+1);
            if( NULL == cmdOsConfigFileName )
            {
                return -1;
            }
            cmdOsConfigFileNameAlloc = GT_TRUE;
            strcpy(cmdOsConfigFileName, argv[i]);
            continue;
        }
        if (strcmp("-noconfig", argv[i]) == 0)
        {
            if( GT_TRUE == cmdOsConfigFileNameAlloc )
            {
                free(cmdOsConfigFileName);
                cmdOsConfigFileNameAlloc = GT_FALSE;
            }
            cmdOsConfigFileName = NULL;
        }
#endif
        /* no need to define environment variable PSEUDO to cause simulation
             * print output on local console
            */
         /* new key "-tty" causes all input/output be on local console */
        if (!strcmp(argv[i], "-tty"))
        {
            setenv("PSEUDO", "/dev/tty", 1);
        }
        
#ifdef SHARED_MEMORY
        if (!strcmp(argv[i], "-slave"))
        {
            isSlaveProcess = GT_TRUE;
        }
#endif
    }

#ifdef CMD_LUA_CLI
    tcgetattr(0, &tc_saved);
    atexit(restore_tty);
#endif
#ifdef SHARED_MEMORY
    if ((GT_TRUE == isSlaveProcess ?
            shrMemInitSharedLibrary()
            : shrMemInitSharedLibrary_FirstClient()) != GT_OK)
    {
        return -1;
    }
    
    if (osStartEngine(argc, (const char**)argv, "appDemo",
              GT_TRUE == isSlaveProcess ? 
                      cmdAppInit /* slave process will not init simulation */
                      : preUserAppInit
        ) != GT_OK)
        return 1;
#else

    if (osStartEngine(argc, argv, "appDemo", preUserAppInit) != GT_OK)
        return 1;
#endif
    return 0;
}



