/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file cmdLuaCLI.c
*
* @brief a LUA CLI implementation
*
* @version   37
********************************************************************************
*/

#include <lua.h>
#include <extUtils/luaCLI/luaCLIEngineCLI.h>

#if !defined(ENV_MAINCMD) && !defined(ENV_POSIX)
#  error "No layer defined. Define one of ENV_MAINCMD, ENV_POSIX"
#endif

/***** Include files ***************************************************/

#ifdef ENV_MAINCMD
# include "cmdLua_os_mainCmd.inc"
#endif
#ifdef ENV_POSIX
# include "cmdLua_os_posix.inc"
#endif

#ifdef ENV_MAINCMD
#include <cmdShell/FS/cmdFS.h>
#include <cmdShell/os/cmdShell.h>
#else
#include <cmdFS.h>
#define cmdOsShell NULL
#endif

typedef void* (*LUACLI_MALLOC_T)(int size);


#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
static GT_VOID *myCpssOsMalloc
(
    IN int size
)
{
    return LUACLI_MALLOC_F(size);
    /*return cpssOsMalloc_MemoryLeakageDbg(size,__FILE__,__LINE__);*/
}
static GT_VOID myCpssOsFree
(
    IN GT_VOID* const memblock
)
{
    LUACLI_FREE(memblock);
    /*cpssOsFree_MemoryLeakageDbg(memblock,__FILE__,__LINE__);*/
}
#endif /*OS_MALLOC_MEMORY_LEAKAGE_DBG*/

/**
* @internal luaCLI_EventLoop function
* @endinternal
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
int luaCLI_EventLoop
(
    IOStreamPTR IOStream,
    int interactive
)
{
#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
    LUA_CLI_MALLOC_STC memMgr = {
        myCpssOsMalloc,
        myCpssOsFree,
        0, 0 };
#else
    LUA_CLI_MALLOC_STC memMgr = {
        (LUACLI_MALLOC_T)LUACLI_MALLOC_F,
        LUACLI_FREE,
        0, 0 };
#endif
    if (!interactive)
    {
        lua_State *L;
        IOStreamPTR IONull = IOStreamCreateNULL(); /* suppress greeting output */
        luaCLIEngine_init(&memMgr, IONull, &cmdFS, osShellGetFunctionByName, &L);
        lua_pushlightuserdata(L, IOStream);
        lua_setglobal(L, "_IOStream");
        IONull->destroy(IONull);
        lua_pushboolean(L, 1);
        lua_setglobal(L, "ignore_all_nice_printings_mode");
        lua_pushboolean(L, 0);
        lua_setglobal(L, "print_nice_enabled");
        lua_pushboolean(L, 0);
        lua_setglobal(L, "myCPSSGenWrapperPausedPrinting");
        IOStream->customPtr = L;
    }
    return luaCLIEngine_EventLoop(
            &memMgr, IOStream, &cmdFS,
            osShellGetFunctionByName, cmdOsShell);
}

