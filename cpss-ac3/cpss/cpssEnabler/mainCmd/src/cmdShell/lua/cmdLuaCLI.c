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


extern GT_U32 luaRestartRequest;
extern GT_U32 luaModeNum;


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
    IOStreamPTR IOStream
)
{
    GT_STATUS   rc;
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

    do {
        /* reset global request to restart lua */
        luaRestartRequest = 0;

        rc = luaCLIEngine_EventLoop(
                &memMgr, IOStream, &cmdFS,
                (LUA_CLI_GETFUNCBYNAME_T)osShellGetFunctionByName, cmdOsShell);

    } while ((rc == GT_OK) && (luaRestartRequest == 1));
    return rc;
}

/* WNC_LUA_CLI_ENHANCE_IOP start */
extern void *malloc(size_t size);
extern void free(void *ptr);

static LUA_CLI_MALLOC_STC memMgr = {
    (LUACLI_MALLOC_T)malloc,
    free,
0, 0 };

GT_STATUS luaCLI_EventLoop_WNC_IOP
(
    IOStreamPTR IOStream,
    void * luaState
)
{
    GT_STATUS   rc;

    do {
        /* reset global request to restart lua */
        luaRestartRequest = 0;

        rc = luaCLIEngine_EventLoop_WNC_IOP(
                 &memMgr, IOStream, &cmdFS,
                 (LUA_CLI_GETFUNCBYNAME_T)osShellGetFunctionByName, cmdOsShell, luaState);

    } while ((rc == GT_OK) && (luaRestartRequest == 1));
    return rc;
}

lua_State * luaCLI_Engine_PreAllocate
(
    IOStreamPTR IOStream
)
{
    lua_State *L = NULL;

    luaCLIEngine_init(&memMgr, IOStream, &cmdFS, (LUA_CLI_GETFUNCBYNAME_T)osShellGetFunctionByName, &L);

    if (L)
    {
        return L;
    }

    return NULL;
}
/* WNC_LUA_CLI_ENHANCE_IOP end */
