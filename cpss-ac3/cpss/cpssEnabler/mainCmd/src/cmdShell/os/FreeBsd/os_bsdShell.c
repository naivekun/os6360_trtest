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
* @file os_bsdShell.c
*
* @brief This file contains OS shell function
*
* @version   2
********************************************************************************
*/
#include <cmdShell/shell/cmdConsole.h>
#include <cmdShell/os/cmdShell.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>


/**
* @internal cmdOsShell function
* @endinternal
*
* @brief   Start OS shell
*         It is assumed that stdout is redirected to current stream
*/
GT_STATUS cmdOsShell(cmdStreamPTR IOStream)
{
    int fd;
    pid_t cpid;
    int status;
    /* run on tty device only (UART, PTY) */
    fd = (IOStream->getFd) ? IOStream->getFd(IOStream) : -1;
    if (fd < 0 || !isatty(fd))
    {
        cmdPrintf("cmdOsShell will run on a TTY only\n");
        return GT_FAIL;
    }

    cpid = vfork();
    if (cpid < 0)
    {
        cmdPrintf("vfork() failed: %s\n", strerror(errno));
    }
    if (cpid == 0)
    {
        /* child */

        /* redirect std */
        dup2(fd, 0);
        dup2(fd, 1);
        dup2(fd, 2);

        execl("/bin/sh", "sh", NULL);
        write(1, "Failed to exec /bin/sh\n", 23);
        _exit(1);
    }
    do {
        if (waitpid(cpid, &status, 0) < 0)
        {
            perror("waitpid()");
            return GT_FAIL;
        }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));

    return GT_OK;
}


