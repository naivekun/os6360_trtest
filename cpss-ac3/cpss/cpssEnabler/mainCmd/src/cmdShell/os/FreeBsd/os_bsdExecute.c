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
* @file os_bsdExecute.c
*
* @brief FreeBsd extended system wrapper layer
*
* this module implements pipe io and standard io redirection routines
*
* @version   4
********************************************************************************
*/

/***** Include files ***************************************************/
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <dlfcn.h>

#include <cmdShell/os/cmdExecute.h>

typedef GT_STATUS (*FUNC_PTR)();


/**
* @internal osShellGetFunctionByName function
* @endinternal
*
* @brief   Lookup executable file symbol table for function
*
* @param[in] funcName                 - null terminated command string
*
* @retval NULL                     - if function is not found
*                                       function pointer
*/
GT_VOIDFUNCPTR osShellGetFunctionByName
(
    IN  const char* funcName
)
{
    return (GT_VOIDFUNCPTR)dlsym(((void *) 0), (char*)funcName);
}

/**
* @internal osShellParseCmdLine function
* @endinternal
*
* @brief   parse command line
*
* @param[in,out] s                        - null terminated command tring, will be modified
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS osShellParseCmdLine
(
    INOUT   char    *s,
    OUT     char*   *cmd,
    OUT     GT_U32  argv[],
    OUT     GT_32  *argc
)
{
    enum {
        ST_SPACE,
        ST_NUMERIC,
        ST_STRING,
        ST_STRING_Q,
        ST_STRING_HEX0,
        ST_STRING_HEX1
    } state = ST_SPACE;
    char *tok_begin = s;
    char *tok_end = s;
    GT_U32  number = 0;
    

    *argc = 0;
    /* skip leading spaces */
    while (*s == ' ' || *s == '\t')
        s++;

    /* get command name */
    *cmd = s;
    while (s && strchr(" \t\n,", *s) == NULL)
        s++;
    if (*s == 0)
    {
        return GT_OK;
    }
    *s++ = 0;
    
    /* parse parameters */
    for (; *s; s++)
    {
        switch (state)
        {
            case ST_SPACE:
                if (strchr(" \t\n,", *s) != NULL)
                    continue;
                argv[*argc] = 0;
                (*argc)++;
                if (*s == '"')
                {
                    state = ST_STRING;
                    tok_begin = s;
                    tok_end = s;
                    argv[*argc-1] = (GT_U32)tok_begin;
                }
                else
                {
                    state = ST_NUMERIC;
                    tok_begin = s;
                }
                continue;
            case ST_NUMERIC:
                if (strchr(" \t\n,", *s) != NULL)
                {
                    *s = 0;
                    argv[*argc-1] = strtoul(tok_begin, 0, 0);
                    state = ST_SPACE;
                    continue;
                }
                continue;
            case ST_STRING:
                if (*s == '"')
                {
                    *tok_end = 0;
                    state = ST_SPACE;
                    continue;
                }
                if (*s == '\\')
                {
                    state = ST_STRING_Q;
                    continue;
                }
                *tok_end++ = *s;
                continue;
            case ST_STRING_Q:
                switch (*s)
                {
                    case 'x': state = ST_STRING_HEX0; number = 0; continue;
                    case 'r': *tok_end++ = '\r'; break;
                    case 't': *tok_end++ = '\t'; break;
                    case 'n': *tok_end++ = '\n'; break;
                    default:
                        *tok_end++ = *s;
                }
                state = ST_STRING;
                continue;
            case ST_STRING_HEX0:
                if (strchr("0123456789abcdef", *s) == NULL)
                {
                    *tok_end++ = (char)number;
                    state = ST_STRING;
                    continue;
                }
                number = (*s > '9')
                    ? (*s + 10 - 'a')
                    : (*s - '0');
                state = ST_STRING_HEX1;
                continue;
            case ST_STRING_HEX1:
                if (strchr("0123456789abcdef", *s) != NULL)
                {
                    number *= 16;
                    number += (*s > '9')
                        ? (*s + 10 - 'a')
                        : (*s - '0');
                }
                *tok_end++ = (char)number;
                state = ST_STRING;
                continue;
        }
    }
    if (state == ST_SPACE)
        return GT_OK;

    if (state == ST_NUMERIC)
    {
        argv[*argc-1] = strtoul(tok_begin, 0, 0);
    }
    else
    {
        /* string */
        *tok_end = 0;
    }
    return GT_OK;
}

/**
* @internal osShellExecute function
* @endinternal
*
* @brief   execute command through OS shell
*
* @param[in] command                  - null terminated  string
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS   osShellExecute
(
    IN  char*   command
)
{
    FUNC_PTR func;
    char copy[128];
    char  * cmd;
    GT_U32 argv[32];
    GT_32 argc;
    GT_STATUS rc;

	strncpy((char*)copy, command, 127);
    copy[127] = '\0';

    if (osShellParseCmdLine(copy, &cmd, argv, &argc) != GT_OK)
    {
        return GT_FAIL;
    }

    func = (FUNC_PTR) osShellGetFunctionByName((char*)cmd);
    if (0 == func)
    {
        return GT_FAIL;
    }
    switch (argc)
    {
        case 0:
            rc = (func)();
            break;
        case 1:
            rc = (func)(argv[0]);
            break;
        case 2:
            rc = (func)(argv[0],argv[1]);
            break;
        case 3:
            rc = (func)(argv[0],argv[1],argv[2]);
            break;
        case 4:
            rc = (func)(argv[0],argv[1],argv[2],argv[3]);
            break;
        case 5:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4]);
            break;
        case 6:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5]);
            break;
        case 7:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5],argv[6]);
            break;
        case 8:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],
                    argv[7]);
            break;
        case 9:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],
                    argv[7],argv[8]);
            break;
        case 10:
            rc = (func)(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],
                    argv[7],argv[8],argv[9]);
            break;
        default:
            return GT_FAIL;
    }
    cmdOsPrintf("return code is %d\n", rc);
    return rc;
}



