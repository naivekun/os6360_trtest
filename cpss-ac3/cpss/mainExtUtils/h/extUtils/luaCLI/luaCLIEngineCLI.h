/*************************************************************************
* luaCLIEngine.h
*
* DESCRIPTION:
*       luaCLI engine
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
**************************************************************************/

#ifndef __luaCLIEngineCLI_h__
#define __luaCLIEngineCLI_h__

/***** Includes  ********************************************************/
#include <extUtils/luaCLI/luaCLIEngine.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal luaCLIEngine_init function
* @endinternal
*
* @brief   Init Lua CLI
*
* @param[in] IOStream
*                                      memstat     - pointer to memstat structure or NULL
*
* @param[out] luaPtr
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
int luaCLIEngine_init
(
    IN LUA_CLI_MALLOC_STC      *memMgr,
    IN IOStreamPTR              IOStream,
    IN genFS_PTR                fs,
    IN LUA_CLI_GETFUNCBYNAME_T  getFunctionByName,
    lua_State                 **luaPtr
);

/**
* @internal luaCLIEngine_EventLoop function
* @endinternal
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
int luaCLIEngine_EventLoop
(
    IN LUA_CLI_MALLOC_STC      *memMgr,
    IN IOStreamPTR              IOStream,
    IN genFS_PTR                fs,
    IN LUA_CLI_GETFUNCBYNAME_T  getFunctionByName,
    IN GT_STATUS (*osShell)(IOStreamPTR IOStream)
);

/* WNC_LUA_CLI_ENHANCE_IOP start */
int luaCLIEngine_EventLoop_WNC_IOP
(
    IN LUA_CLI_MALLOC_STC      *memMgr,
    IN IOStreamPTR              IOStream,
    IN genFS_PTR                fs,
    IN LUA_CLI_GETFUNCBYNAME_T  getFunctionByName,
    IN GT_STATUS (*osShell)(IOStreamPTR IOStream),
    IN void * luaState
);
/* WNC_LUA_CLI_ENHANCE_IOP end */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __luaCLIEngineCLI_h__ */

